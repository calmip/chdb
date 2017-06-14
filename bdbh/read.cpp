
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <sstream>
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "read.hpp"
using bdbh::Read;

/** For each file and directory in the command line, call __Exec
    The files are expanded if necessary

*/

void Read::Exec() throw(BdbhException,DbException)
{
    Mdata mdata;
    
    vector<Fkey> fkeys = prm.GetFkeys();

    for (unsigned int i=0; i<fkeys.size(); ++i)
    {
        // If CAT: do not create the intermediate directories, do not expand wildcards
        if (prm.GetCommand()==BDBH_CAT)
        {
            if (!fkeys[i].IsLeaf())
                continue;

            __Exec(fkeys[i]);

            if (_IsSignalReceived())
                break;
        }
        else
        {       			
            // Expand wildcards if any
            if (fkeys[i].GetKey().find('*')!= string::npos)
            {
                // expand the wilcards, generating a vector of strings
                vector<string> k_exp = _ExpandWildcard(fkeys[i].GetKey());
                
                // transform this vector in vector of Fkeys 
                vector<Fkey> fkeys_exp;
                while(k_exp.size() !=0)
                {
                    StripLeadingString(Fkey::root,k_exp.back());
                    Fkey fk(k_exp.back(),fkeys[i].IsLeaf(),fkeys[i].IsRecurs());
                    k_exp.pop_back();
                    fkeys_exp.push_back(fk);
                }
                
                // For each new fkey, call __Exec
                for (unsigned int j=0; j<fkeys_exp.size();++j)
                {
                    __Exec(fkeys_exp[j]);
                    if (_IsSignalReceived())
                        break;

                }
            }
            else
            {
                __Exec(fkeys[i]);
            }
            if (_IsSignalReceived())
                break;
        };
    }
      
    // For each extracted directory, fix its permissions
    mode_t umsk = umask(0);   // retrieving the umask without modifying it
    umask(umsk); 
    for (vector<DirectoryEntry>::const_iterator i=directories.begin(); i != directories.end(); ++i)
    {
        chmod(i->name.c_str(),i->mdata.mode&~umsk);
        __RestoreTime(i->name,i->mdata);
        chown(i->name.c_str(), i->mdata.uid, i->mdata.gid);
    }
    
    if (_IsSignalReceived())
        cerr << "Interrupted by user\n";
}

/** Read the fkey with the data, and call __ExecDir, __ExecFile or __ExecSymLink

\param fkey The fkey to extract
        
*/

void Read::__Exec(const Fkey& fkey) throw(BdbhException,DbException)
{
    Mdata mdata;
    string key = fkey.GetKey();
    string nkey;
    string file_name = fkey.GetFileName();
    //cerr << "coucou " << key << " *** " << fkey.IsRecurs() << "\n";
    
    // If we are not processing a leaf AND if the directory is already created, nothing to do
    if (!fkey.IsLeaf() && _IsInFs(file_name.c_str()))
        return;

	// If key is "", create the directory
	if (key == "") {
		int rvl = mkdir(file_name.c_str(),0700);      // Temporary relax mode
		if (rvl==0) {
			return;
		} else if (rvl==-1 && errno!=EEXIST) {
			throw BdbhException(Fkey::directory,errno);
		};
	}

    // Read the key and the data
    int rvl=_ReadKeyDataCursor(key,nkey,mdata,0,true,true);
    if (rvl==DB_NOTFOUND)
    {
        exit_status=BDBH_ERR_NF;
        prm.Log("key " + key + " not found",cerr);
    }
    else
    {
        if (S_ISDIR(mdata.mode))
        {
            if (prm.GetCommand()==BDBH_CAT)
            {
                exit_status=BDBH_ERR_DR;
                prm.Log("could not cat key " + key + ": it is a directory",cerr);
                return;
            }
            else
            {
                __ExecDir(fkey,mdata);
            }
        }
        else if (S_ISREG(mdata.mode))
        {
            __ExecFile(fkey,mdata);
        }
        else if (S_ISLNK(mdata.mode))
        {
            if (prm.GetCommand()==BDBH_CAT)
            {
                exit_status=BDBH_ERR_DR;
                prm.Log("could not cat key " + key + ": it is a symlink",cerr);
                return;
            }
            else
            {
                __ExecSymLink(fkey);
            }
        }
        else
        {
            throw(BdbhException("ERROR - unsupported data type (may be data are corrupted ?)"));
        }
    }
    
}

/** (may be recursively) extract the directory passed by parameter

    When __ExecDir is called, the key has already been read and should point to a directory.
    mkdir is called and the directory is kept in a stack (for the further chmod)
    If recursivity is asked (see fkey), and if it is not too deep (--level), 
    __ExecFile, __ExecSysmLink or __ExecDir is called

\param fkey The fkey to extract
\param mdata The corresponding metadata
        
*/
void Read::__ExecDir(const Fkey& fkey, Mdata mdata) throw(BdbhException,DbException)
{

    // Make the directory
    const string& file_name = fkey.GetFileName();
    const string& key = fkey.GetKey();
    
    int rvl;    
    if (key!="")
    {
        // Call mkdir 
        rvl = mkdir(file_name.c_str(),0700);      // Temporary relax mode
        if (rvl==-1 && errno!=EEXIST)
        {
            throw BdbhException(file_name,errno);
        }
        else if (rvl==0)
        {
            // Directory: we shall restore later the properties from the metadata
            // else it will be imposible to retrieve a read-only hierarchy
            DirectoryEntry d(file_name,mdata);
            directories.push_back(d);
            prm.Log("file " + file_name + " extracted (directory)",cerr);
        };
    }
    
    // If recursivity wanted and if not too deep, explore this directory
    if (fkey.IsRecurs())
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
                string msg = "Problem in the database: the special file >" + marker +"< does not exsit"; 
                throw(BdbhException(msg));
            }
            
            while(rvl!=DB_NOTFOUND)
            {
                rvl=_ReadKeyDataCursor(key,nkey,mdata,lvl,false,true);
                if (rvl != DB_NOTFOUND)
                {
                    // Remove root if necessary
                    StripLeadingString(Fkey::root,nkey);
                    if (S_ISDIR(mdata.mode))
                    {
                        Fkey nfk(nkey,fkey.IsLeaf(),fkey.IsRecurs());
                        __ExecDir(nfk,mdata);
                    }
                    else if (S_ISREG(mdata.mode))
                    {
                        Fkey nfk(nkey,false,false);
                        __ExecFile(nfk, mdata);
                    } 
                    else if (S_ISLNK(mdata.mode))
                    {
                        Fkey nfk(nkey,false,false);
                        __ExecSymLink(nfk);
                    }
                    else
                    {
                        throw(BdbhException("ERROR - unsupported data type (may be data are corrupted ?)"));
                    }
                }
                if (_IsSignalReceived())
                    break;
            }
        }
    }
    return;
}

/** Extract the file passed by parameter

    When __ExecFile is called, the key has already been read and should point to a regular file
    The data area is written to the file

\param fkey The fkey to extract
\param mdata The corresponding metadata

*/
void Read::__ExecFile(const Fkey& fkey, Mdata& mdata) throw(BdbhException)
{
    string file_name = fkey.GetFileName();
    if (prm.GetCommand()==BDBH_CAT)
    {
        cout.write((const char*)GetDataBfr().GetData(),GetDataBfr().GetSize());
    }
    else if (prm.GetOverWrite() || !_IsInFs(file_name.c_str()))
    {
        // Write back the data to the file, may be overwriting an existing file
        mode_t mode = mdata.mode; 
        
        // check we are not overwriting ourself
        _IsDbItself(file_name.c_str());

        // Create and write the file
        File f(file_name.c_str(),O_WRONLY|O_CREAT,mode);
        int fd = f.GetFd();
        if (fd==-1)
            throw(BdbhException(file_name,f.GetErr()));
        
        int rvl = write(fd,GetDataBfr().GetData(),GetDataBfr().GetSize());
        if (rvl==-1)
            throw(BdbhException(file_name,f.GetErr()));
        
        // Restore properties from the metadata            
        __RestoreTime(file_name,mdata);
        fchown(fd, mdata.uid, mdata.gid);
        
        // message in verbose mode
        prm.Log("file " + file_name + " extracted (regular file)",cerr);
    }
    else
    {
        prm.Log("could not extract file " + file_name + ": --overwrite not specified",cerr);
    }
}

/** Extract the symlink passed by parameter

    When __ExecFile is called, the key has already been read and should point to a symlink
    The link is created, the pointed file is in the data area
    There is no need for metadata

\param fkey The fkey to extract

*/
void Read::__ExecSymLink(const Fkey& fkey) throw(BdbhException)
{
    const char* link_name = fkey.GetFileName().c_str();
    GetDataBfr().SetSize(GetDataBfr().GetSize()+1);
    char* link_dest = (char*) GetDataBfr().GetData();
    link_dest[GetDataBfr().GetSize()-1] = '\0';         // Add a termination \0
    int rvl = symlink(link_dest,link_name);
    if (rvl==-1 && errno!=EEXIST)
        throw(fkey.GetFileName(),errno);
    prm.Log("file " + fkey.GetFileName() + " extracted (symlink)",cerr);
}


/** Using the metadata passed by prm, restore the access and modification times of the file

\param file_name The file name
\param mdata The corresponding metadata
*/
void Read::__RestoreTime(const string& file_name, const Mdata& mdata)
{
    
    struct utimbuf buf;
    buf.actime = mdata.atime;
    buf.modtime= mdata.mtime;
    utime(file_name.c_str(),&buf);
}


