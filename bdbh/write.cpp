
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <sstream>
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "write.hpp"

using bdbh::Write;

/**
For each file and directory in the command line, call __Exec
*/
void Write::Exec() throw(BdbhException,DbException)
{
    const vector<Fkey>& fkeys = prm.GetFkeys();

    for (unsigned int i=0; i < fkeys.size(); ++i)
    {
		// If the key is empty, nothing to do (could happen with the --directory switch)
		if (fkeys[i].GetKey().size()!=0) {
			__Exec(fkeys[i]);
		}
		if (_IsSignalReceived())
		{
			cerr << "Interrupted by user\n";
			break;
		}
	};
}

/** Write a file to the database

Read the metadata and the data from the file and write them to the database,
may be overwriting the old data. Call __ExecSymLink and __ExecDir if the file is a symlink or a directory

*/
void Write::__Exec(const Fkey& fkey)
{
    string key = fkey.GetKey();
    string file_name= fkey.GetFileName();
/*    cerr << "coucou _Exec key=" << fkey.GetKey() << " file=" << fkey.GetFileName() << " leaf=" << fkey.IsLeaf() << "\n";*/
    
	// If file_name empty, we just have to create a "directory"
	if (file_name=="") {
		_Mkdir(fkey);
		return;
	}

    // If we are not processing a leaf AND if it is already in database, nothing to do
    if (!fkey.IsLeaf() && _IsInDb(key.c_str()))
        return;
    
    // Open the file read only, but do not open if it is a symlink
    File f(file_name.c_str(),O_RDONLY|O_NOFOLLOW);
    int fd = f.GetFd();
    
    // It is not a symlink 
    if (fd!=-1)
    {
        // Build the mdata object
        Mdata mdata;
        struct stat st_bfr;
        int rvl = fstat(fd,&st_bfr);
        if (rvl==-1)
            throw(BdbhException(file_name,errno));
/*        
        if (st_bfr.st_size >= prm.GetMaxFileSize())
        {
            ostringstream tmp;
            tmp << "The file " + file_name + " is too large (" << st_bfr.st_size << ">" << prm.GetMaxFileSize() <<") to be added to the database";
            throw(BdbhException(tmp.str().c_str()));
        }
*/        
        // Check we are not storing the database in the database (!)
        _IsDbItself(st_bfr);

        mdata.mode  = st_bfr.st_mode;
        mdata.uid   = st_bfr.st_uid;
        mdata.gid   = st_bfr.st_gid;
        mdata.size  = st_bfr.st_size;
        mdata.atime = st_bfr.st_atime;
        mdata.mtime = st_bfr.st_mtime;
        
        // Is it a regular file ? If yes, we process it
        if (S_ISREG(st_bfr.st_mode)) 
        {

            if (st_bfr.st_size >= prm.GetMaxFileSize())
            {
                ostringstream tmp;
                tmp << "The file " + file_name + " is too large (" << st_bfr.st_size << ">" << prm.GetMaxFileSize() <<") to be added to the database";
                throw(BdbhException(tmp.str().c_str()));
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
                
                // Alloc a buffer to store the whole file in memory
                size_t data_len = st_bfr.st_size;
                GetDataBfr().SetSize(data_len);
                
                // Read the file and store it into the buffer
                rvl = read(fd,GetDataBfr().GetData(),data_len);
                if (rvl==-1)
                    throw(BdbhException(file_name,errno));
                
                // Store the (key,data) pair inside the database
                // Check the return value: there could be a race condition if several processes try to add
                // the same files and we are NOT in --overwrite mode
                // In --overwrite mode, this race condition cannot be detected
                int rvl = _WriteKeyData(key,mdata,true,prm.GetOverWrite());
                if (rvl !=0)
                {
                    prm.Log("could not update key " + key + ": Db::put returned an error",cerr);
                }
                else
                {
                    // Update info_data - If overwriting some file, we update 2 times:
                    // -1 time for removing the old version
                    // -1 time for adding the new version, may be modifying the max data size
                    if (is_in_db)
                    {
                        __UpdateDbSize(-mdata_in_db.size,-mdata_in_db.csize,0,0,0);
                        __UpdateDbSize(mdata.size,mdata.csize,0,0,0);
                    }
                    else
                    {
                        __UpdateDbSize(mdata.size,mdata.csize,key.size(),1,0);
                    }
                }                
                // message in verbose mode
                prm.Log("key " + key + " updated (regular file)",cerr);
            }
            else
            {
                exit_status=BDBH_ERR_OW;
                prm.Log("could not update key " + key + ": --overwrite not specified",cerr);
            }
        }
        // Is it a directory ? If yes, we call __ExecDir
        else if (S_ISDIR(st_bfr.st_mode))
        {
            __ExecDir(fkey, mdata);
        }
        // It is something else: not stored !
        else
        {
            throw(BdbhException("not a directory/symlink/regular file"));
        }
    }
    else
    {
        // It is a symbolic link - Call the specialized function
        if (errno==ELOOP)
        {
            if (_IsInDb(key.c_str()))
            {
                exit_status=BDBH_ERR_DR;
                prm.Log("could not update key " + key + ": A link cannot overwrite anything",cerr);
                return;
            }
            __ExecSymLink(fkey);
        }
        else
        {
            throw(BdbhException(file_name,errno));
        };
    };
}

/** Write the directory to the database, may be with recursive calls

  - Write the metadata only to the database (no data to write)
  - If recursivity is on, open and read the directory, calling __Exec for each entry
  
  \param fkey
  \param[in,out] mdata The metadata, may be changed
*/
void Write::__ExecDir(const Fkey& fkey, Mdata & mdata)
{
    const string& file_name = fkey.GetFileName();
    const string& key = fkey.GetKey();
    // cerr << "coucou _ExecDir key=" << fkey.GetKey() << " file=" << fkey.GetFileName() << " leaf=" << fkey.IsLeaf() << "\n";
    
    // The directory is never overwritten: if already exists, we do not add the directory
    if (!_IsInDb(key.c_str(),mdata))
    {
        // Alloc an empty buffer, so that only the metadata will be stored
        GetDataBfr().SetSize(0);
        int rvl = _WriteKeyData(key,mdata,false,false);
        if (rvl!=0)
        {
            prm.Log("could not update key " + key + ": Db::put returned an error",cerr);
        }
        else
        {
            __UpdateDbSize(0,0,key.size(),0,1);          // adding a directory to the count
            prm.Log("key " + key + " updated (directory)",cerr);
            
            // Create the special empty file direc
            string k = key + DIRECTORY_MARKER;
            mdata.mode = S_IFREG | 0666;
            mdata.size = 0;
            GetDataBfr().SetSize(0);
            
            _WriteKeyData(k,mdata,false,false);
            __UpdateDbSize(0,0,k.size(),0,0);          // Do not count the file, only the key size
        }
    }

    // If the flag recurs is set we look inside the directory
    if (fkey.IsRecurs())
    {
        Dir d(file_name.c_str());
        DIR* dir = d.GetDir();
        if (dir==NULL)
            throw(BdbhException(file_name,d.GetErr()));
       
        struct dirent* dir_entry=NULL;
        do
        {
            dir_entry = readdir(dir);
            if (dir_entry != NULL)
            {
                if (dir_entry->d_name[0]=='.' && dir_entry->d_name[1]=='\0')  // Do not process directory .
                    continue;
                if (dir_entry->d_name[0]=='.' && dir_entry->d_name[1]=='.' && dir_entry->d_name[2]=='\0')  // Do not process directory ..
                    continue;
                
                //string new_file_name = file_name + '/' + dir_entry->d_name;
				//cerr << "coucou file name=" << file_name << " / " <<  dir_entry->d_name << "\n";
				Fkey new_fkey = fkey;
				new_fkey.Down(dir_entry->d_name);
                __Exec(new_fkey);
            }
            if (_IsSignalReceived())
                break;
            
        } while (dir_entry!=NULL);
    };
}

/** Write the symlink to the database

    We never follow the symbolic links, they are instead stored to the database, with their associated metadata
*/
void Write::__ExecSymLink(const Fkey& fkey)
{
    string f = fkey.GetFileName();
    string k = fkey.GetKey();
    
    // Build the mdata object
    Mdata mdata;
    struct stat st_bfr;
    int rvl = lstat(f.c_str(),&st_bfr);
    if (rvl==-1)
        throw(BdbhException(f,errno));
    
    mdata.mode  = st_bfr.st_mode;
    mdata.uid   = st_bfr.st_uid;
    mdata.gid   = st_bfr.st_gid;
    mdata.size  = st_bfr.st_size;
    mdata.atime = st_bfr.st_atime;
    mdata.mtime = st_bfr.st_mtime;
    
    // Read the link and store it in the data buffer
    GetDataBfr().SetSize(-1);       // at least 1 Mb, should be OK
    rvl=readlink(f.c_str(), (char*) GetDataBfr().GetData(),5000/*data_bfr.GetSize()*/);
    if (rvl==-1)
        throw(BdbhException(f,errno));
    GetDataBfr().SetSize(rvl);      // Resize the buffer to the number of characters read
    
    // Store the (key,data) pair inside the database
    _WriteKeyData(k,mdata,true,prm.GetOverWrite());
    
    // Update the size
    __UpdateDbSize(mdata.size,mdata.csize,k.size(),1,0);
    
    prm.Log("key " + k + " updated (symlink)",cerr);
}


