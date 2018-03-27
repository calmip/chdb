/** This file is part of bdbh software
 * 
 * bdbh helps users to store little files in an embedded database
 *
 * bdbh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2010-2014    L I P M
 *  Copyright (C) 2015-2018    C A L M I P
 *  bdbh is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bdbh.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <sstream>
#include <iomanip>
#include <list>
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "ls.hpp"
using bdbh::Mdata;
using bdbh::Ls;
using bdbh::LsPrint;


/** For each key specified in the parameters, read its metadata and update the observer
 */

void Ls::Exec()
{
    Mdata mdata;
    
	if (__obs==NULL) {
		throw(logic_error("Ls: The observer should be configured at this point"));
	}

    vector<Fkey> fkeys = prm.GetFkeys();
    if (fkeys.size()==0)
        fkeys.push_back(Fkey("*",true,true));         // If no key, add a wildcard and mark the key as recursive

    for (unsigned int i=0; i<fkeys.size(); ++i)
    {
        // Skip the parameters which are not leaves
        if (!fkeys[i].IsLeaf())     
            continue;
        
        // Expand wildcards if any
        if (fkeys[i].GetKey().find('*')!= string::npos)
        {
            // expand the wilcards, generating a vector of strings
            vector<string> k_exp = _ExpandWildcard(fkeys[i].GetKey());

            // For each new key, call __Exec
            for (unsigned int j=0; j<k_exp.size();++j)
            {
                __Exec(k_exp[j],fkeys[i].IsRecurs());
                if (_IsSignalReceived())
                    break;
            }
        }
        else
        {
            __Exec(fkeys[i].GetKey(),fkeys[i].IsRecurs());
        }
        if (_IsSignalReceived())
        {
            cerr << "Interrupted by user\n";
            break;
        }
    };
}

/** \brief If the key is a directory, call __ExecDir If not, update the observer
	\param key The key to list
	\param is_recurs If true, recursivity is applied for directories
        
*/

void Ls::__Exec(const string& key, bool is_recurs)
{
    Mdata mdata;
    string nkey;

    int rvl=_ReadKeyDataCursor(key,nkey,mdata,0,true);
    if (rvl==DB_NOTFOUND)
    {
        exit_status=BDBH_ERR_NF;
        prm.Log("key " + key + " not found",cerr);
    }
    else
    {
        if (S_ISDIR(mdata.mode)&&is_recurs)
        {
            __ExecDir(key,mdata,is_recurs);
        }
        else
        {
	    __obs->Update(key,mdata);
            //__List(key,mdata,cout);
        }
    }
}

/** (may be recursively) List the directory passed by parameter

    When __ExecDir is called, the key has already been read. It is thus listed, and
    files present in this directory are listed, __Execdir is recursively called for
    subdirectories. The search may be level-limited, however

	\param key The key to list
	\param mdata The corresponding metadata
	\param is_recurs If true, recursivity is applied for directories
        
*/
                
void Ls::__ExecDir(const string& key, Mdata mdata, bool is_recurs)
{
    __obs->Update(key,mdata);
    //__List(key,mdata,cout);
    if (is_recurs)
    {
	int rvl=0;
	int lvl = CountLevel(key.c_str())+1;
	if (_IsNotTooDeep(lvl)!=0)
	{

	    // This is used for sorting files before printing them
	    list<finfo> lf;

	    string nkey;
	    string marker = key+DIRECTORY_MARKER;
	    rvl=_ReadKeyDataCursor(marker,nkey,mdata,lvl,true);
	    if (rvl==DB_NOTFOUND)
            {
		string msg = "Problem in the database: the special file #" + marker +"# does not exsit"; 
		throw(BdbhException(msg));
            }
            
	    while(rvl!=DB_NOTFOUND)
	    {
		rvl=_ReadKeyDataCursor(key,nkey,mdata,lvl,false);
		if (rvl != DB_NOTFOUND)
                {
		    if (S_ISDIR(mdata.mode))
                    {
			__ExecDir(nkey,mdata,is_recurs);
                    }
		    else
                    {
			if ( prm.GetSorted() )
			{
			    lf.push_back(finfo(nkey,mdata));
			}
			else
			{
			    __obs->Update(nkey,mdata);
			    //__List(nkey,mdata,cout);
			}
                    }
                }

		if (_IsSignalReceived())
		    break;
            }
	    if ( prm.GetSorted() )
	    {
		lf.sort();
		if ( !prm.GetSortedReverse() )
		{
		    for ( list<finfo>::iterator i=lf.begin(); i!=lf.end();++i)
		    {
			__obs->Update(i->name,i->mdata);
			//__List(i->name,i->mdata,cout);
		    }
		}
		else
		{
		    for ( list<finfo>::reverse_iterator i=lf.rbegin(); i!=lf.rend();++i)
		    {
			__obs->Update(i->name,i->mdata);
			//__List(i->name,i->mdata,cout);
		    }
		}
	    }
        }
    }
    return;
}

/** Print the metadata of this key 

	\param name The key
	\param mdata The metadata

*/
void LsPrint::Update(string name, const Mdata& mdata)
{
    string root = prm.GetRoot();
    StripTrailingSlash(root);
    
    // If root not stripped or if nothing to display, return
    if (!StripLeadingStringSlash(root,name) || name.length()==0)
        return;
    
    // Display the name
    if (prm.GetLongList())
    {
	os << setw(8) << setfill(' ') << mdata.ino;
	
        if (S_ISREG(mdata.mode))
            os << " f";
        else if (S_ISDIR(mdata.mode))
            os << " d";
        else 
            os << " l";
        
        int m = 07777&mdata.mode;
        os << setw(5) << setfill(' ') << oct << m << "  ";
        
        struct passwd* pwd = getpwuid(mdata.uid);
        if (pwd==NULL)
            os << setw(10) << mdata.uid;
        else
            os << setw(10) << pwd->pw_name;
        
        struct group* grp = getgrgid(mdata.gid);
        if (grp==NULL)
            os << setw(10) << mdata.gid;
        else
            os << setw(10) << grp->gr_name;
        
        if (mdata.cmpred)
        {
            os << dec << setw(12) << mdata.csize << " " << setw(9) << mdata.size ;
        }
        else
        {
            os << dec << setw(22) << mdata.size ;
        }

        os << "  " << Time(mdata.mtime) << name << "\n";
    }
    else
    {
        cout << name << '\n';
    };
}

