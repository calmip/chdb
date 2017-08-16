
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <sstream>
#include <algorithm>
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "put.hpp"

using bdbh::Put;
using bdbh::Mkdir;

/**
    @brief executing put: Reverse the order of the keys to treat the leaf at the end
*/
void Put::Exec()
{
    vector<Fkey> fkeys = prm.GetFkeys();
    reverse(fkeys.begin(),fkeys.end());
    
    for (unsigned int i=0; i < fkeys.size(); ++i)
    {
        __Exec(fkeys[i]);
        if (fkeys[i].IsLeaf())
            break;                  // Ignore the other keys, if any
        if (_IsSignalReceived())
        {
            cerr << "Interrupted by user\n";
            break;
        }
        
    };
}

/** Read stdin and write 

Read the data from stdin or from the --value parameter and write them to the database,
may be overwriting the old data. Call __ExecDir if the file is a symlink or a directory

*/
void Put::__Exec(const Fkey& fkey)
{
    string key = fkey.GetKey();
    //Coucou c("__Exec "+key);
    //cerr << "coucou " << fkey.GetKey() << "  " << fkey.IsLeaf() << "\n";
    
    // Init a new mdata
    Mdata mdata;
    mdata.uid = getuid();
    mdata.gid = getgid();
    mdata.size = 0;
    mdata.csize = 0;
    mdata.cmpred = false;
    
    // time information = get the current time
    timeval tv;
    gettimeofday(&tv,NULL);
    mdata.atime = tv.tv_sec;
    mdata.mtime = tv.tv_sec;
    
    // If we are not processing a leaf AND if it is already in database, nothing to do
    if (!fkey.IsLeaf() && _IsInDb(key.c_str()))
        return;
    
    // If we are NOT processing a leaf, it is a directory (even if the directory does not really exist)
    if (!fkey.IsLeaf())
    {
        mdata.mode = S_IFDIR | 0770;    // It is a directory, rw-rw-rw (will be changed by umask if retrieved to a file)
        _Mkdir(fkey,mdata);
    }
    // If we are processing a leaf, it is a file
    else
    {
        mdata.mode = S_IFREG | 0666;    // It is a file, rw-rw-rw (will be changed by umask if retrieved to a file)
        
        // Get value from the --value switch
        if (prm.IsValueAvailable())
        {
            GetDataBfr().SetSize(prm.GetValue().length());
            memcpy(GetDataBfr().GetData(),prm.GetValue().c_str(),prm.GetValue().length());
            mdata.size = GetDataBfr().GetSize();
        }
            
        // Read stdin and store it into the buffer 
        // WARNING - It can be silently trunked !!!
        else
        {
            GetDataBfr().SetSize(-1);
            int cnt = read(fileno(stdin),GetDataBfr().GetData(),GetDataBfr().GetSize());
            if (cnt==-1)
                throw BdbhException("stdin",errno);
            GetDataBfr().SetSize(cnt);
            mdata.size = cnt;
        }
        
        Mdata mdata_in_db;
        bool is_in_db = _IsInDb(key.c_str(),mdata_in_db);
        if (prm.GetOverWrite() || !is_in_db)
        {

            // We refuse to overwrite a directory or a link
            if (is_in_db && !S_ISREG(mdata_in_db.mode))
            {
                exit_status=BDBH_ERR_DR;
                prm.Log("could not update key " + key + ": Cannot overwrite a directory or a link",cerr);
                return;
            }

            // Store the (key,data) pair inside the database
            // Reuse the inode if overwrite, or ask for a new inode number
            if (!is_in_db) {
                mdata.ino = _NextInode();
            } else {
                mdata.ino = mdata_in_db.ino;
            }
            _WriteKeyData(key,mdata);
            
            // Update info_data - If overwriting some file, we update 2 times:
            // -1 time for removing the old version
            // -1 time for adding the new version, may be modifying the max data size
            if (is_in_db)
            {
                _UpdateDbSize(-mdata_in_db.size,-mdata_in_db.csize,0,0,0,0);
                _UpdateDbSize(mdata.size,mdata.csize,0,0,0,0);
            }
            else
            {
                _UpdateDbSize(mdata.size,mdata.csize,key.size(),1,0,0);
            }

            // message in verbose mode
            prm.Log("key " + key + " updated (regular file from stdin)",cerr);
        }
        else
        {
            exit_status=BDBH_ERR_OW;
            prm.Log("could not update key " + key + ": --overwrite not specified",cerr);
        }
    }
}

/**
Call__Exec for each key, creating directories 
*/
void Mkdir::Exec()
{
    const vector<Fkey>& fkeys = prm.GetFkeys();
    
    for (unsigned int i=0; i < fkeys.size(); ++i)
    {
        _Mkdir(fkeys[i]);
        if (_IsSignalReceived())
        {
            cerr << "Interrupted by user\n";
            break;
        }
        
    };
}

