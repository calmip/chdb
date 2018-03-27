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
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "chmod.hpp"

using bdbh::Chmod;

/** \brief For each key specified in the parameters, read and change its metadata
*/

void Chmod::Exec()
{
    //Mdata mdata;
    mode_t to_mode;
    const mode_t mask = 0007777;        // The permission bits

    // Read the mode: this string represents an octal number ...
    // or dooo (only for directories) or fooo (only for regular files)

    string mtmp =  prm.GetMode();
    mode_t f_sel=0;                     // Selectionning directories of files or both
    if (mtmp[0]!='d')         
        f_sel |= S_IFREG;
    if (mtmp[0] != 'f')
        f_sel |= S_IFDIR;
    if (mtmp[0]=='d' || mtmp[0]=='f')   // d755 ==> 755
        mtmp.erase(0,1);
    
    istringstream tmp(mtmp);
    tmp >> oct >> to_mode;
    to_mode &= mask;            // Keep only the permission bits
    
   
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
                __Exec(k_exp[j],fkeys[i].IsRecurs(),f_sel, to_mode);
                if (_IsSignalReceived())
                    break;
            }
        }
        else
        {
            __Exec(fkeys[i].GetKey(),fkeys[i].IsRecurs(),f_sel, to_mode);
        }
        if (_IsSignalReceived())
        {
            cerr << "Interrupted by user\n";
            break;
        }
    };
}

/** If the key is a directory, call __ExecDir If not, call __List

\param key The key to list
\param is_recurs If true, recursivity is applied for directories
\param f_sel The flag indicating if the mode must be changed for files, directories or both
\param to_mode the new mode
        
*/

void Chmod::__Exec(const string& key, bool is_recurs,mode_t f_sel, mode_t to_mode)
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
            __ExecDir(key,mdata,is_recurs,f_sel, to_mode);
        }
        else
        {
            __Mode(key,mdata,f_sel, to_mode);
        }
    }
}

/** (may be recursively) change the mode of the directory passed by parameter

    When __ExecDir is called, the key has already been read. __Mode is thus called,
    and it is also called for files present in this directory are listed.
    __Execdir is called for subdirectories. The search may be level-limited, however

\param key The key
\param mdata The corresponding metadata
\param is_recurs If true, recursivity is applied for directories
\param f_sel The flag indicating if the mode must be changed for files, directories or both
\param to_mode the new mode
        
*/
                
void Chmod::__ExecDir(const string& key, Mdata mdata, bool is_recurs, mode_t f_sel, mode_t to_mode)
{
    __Mode(key,mdata,f_sel,to_mode);
    if (is_recurs)
    {
        int rvl=0;
        int lvl = CountLevel(key.c_str())+1;
        if (_IsNotTooDeep(lvl)!=0)
        {
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
                        __ExecDir(nkey,mdata,is_recurs,f_sel,to_mode);
                    }
                    else if (S_ISREG(mdata.mode))
                    {
                        __Mode(nkey,mdata,f_sel,to_mode);
                    }
                }
                if (_IsSignalReceived())
                    break;
            }
        }
    }
    return;
}
 

/** Change the mode of the key

\param key The key to change
\param mdata The corresponding metadata
\param f_sel The flag indicating if the mode must be changed for files, directories or both
\param to_mode the new mode

\todo Using the retrieved cursor here would be a better idea

*/

void Chmod::__Mode(const string& key, Mdata mdata, mode_t f_sel, mode_t to_mode)
{
    const mode_t mask = 0007777;        // The permission bits
    
    if (f_sel&mdata.mode)
    {
        // Change mode
        mdata.mode &= ~mask;    // permission bits set to 0
        mdata.mode |= to_mode;  // permission bits set to the wanted mode
        
        // Write metadata only
        _WriteKeyData(key, mdata, false);
        prm.Log((string)"key " + key + ": mode changed",cerr);
    }
}

