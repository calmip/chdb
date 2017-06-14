
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
#include "rm.hpp"

using bdbh::Rm;

/** For each key in the parameters, call __remove if possible

   It is possible to remove the key only if:
      - it is a leaf
      - it is a file or a symlink
      - it is an empty directory
*/
/** For each key specified in the parameters, read its metadata and call __List
*/

void Rm::Exec() throw(BdbhException,DbException)
{
    Mdata mdata;
    
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

/** If the key is a directory, call __ExecDir If not, call __List

\param key The key to list
\param is_recurs If true, recursivity is applied for directories
        
*/

void Rm::__Exec(const string& key, bool is_recurs) throw(BdbhException,DbException)
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
        if (S_ISDIR(mdata.mode))
        {
            __ExecDir(key,mdata,is_recurs);
        }
        else
        {
            __Remove(key,mdata,CountLevel(key.c_str()));
            __UpdateDbSize(-mdata.size,-mdata.csize,-nkey.size(),-1,0);
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
                
void Rm::__ExecDir(const string& key, Mdata mdata, bool is_recurs) throw(BdbhException,DbException)
{
    if (is_recurs)
    {
        int rvl=0;
        int lvl = CountLevel(key.c_str())+1;
        string nkey;
        string marker = key+DIRECTORY_MARKER;
        rvl=_ReadKeyDataCursor(marker,nkey,mdata,lvl,true);
        if (rvl==DB_NOTFOUND)
        {
            string msg = "Problem in the database: the special file #" + marker +"# does not exist"; 
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
                    __Remove(nkey,mdata,lvl);
                    __UpdateDbSize(-mdata.size,-mdata.csize,-nkey.size(),-1,0);
                }
            }
            // If a signal is received, return instead of breaking: do not remove marker and directory
            if (_IsSignalReceived())
                return;
        }
        
        // Everything in this directory was removed, position the cursor to the marker again
        rvl=_ReadKeyDataCursor(marker,nkey,mdata,lvl,true);
        if (rvl==DB_NOTFOUND)
        {
            string msg = "Error: the special file #" + marker +"# disappeared"; 
            throw(BdbhException(msg));
        }
        
        // Remove the marker, then the directory
        __Remove(marker,mdata,lvl);
        __Remove(key,mdata,lvl-1);
       
        // Update the global info
        __UpdateDbSize(0,0,-key.size()-marker.size(),0,-1);
        
    }
    return;
}

/** Remove the key passed by parameter 

If the key if home/manu, All the home/manu/ keys will be removed 
\param key The key
\param mdata The metadata (not used)
\param lvl The level of he key (leading to the correct cursor)

*/

void Rm::__Remove(const string& key, Mdata mdata, int lvl) throw(BdbhException,DbException)
{
    // Remove the key, using the cursor
    int rvl=0;
    rvl = _RemoveUsingCursor(lvl);
    if (rvl != 0)
    {
        string msg = "Error: the directory " + key +" could not be removed"; 
        throw(BdbhException(msg));
    }

    // Log if verbose mode    
    prm.Log("key " + key + " removed",cerr);
    
    return;
}


