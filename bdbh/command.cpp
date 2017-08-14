
#include <iostream>
using namespace std;

#include <stdlib.h>
#include <signal.h>
#include <db_cxx.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sstream>
#include <time.h>
#include <iomanip>
#include <zlib.h>
#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"

using bdbh::Buffer;
using bdbh::Command;
using bdbh::BerkeleyDb;
using bdbh::InfoData;
using bdbh::Time;

//int Coucou::level=0; See the class Coucou, used for debugging

int Command::signal_received = 0;

/** class initialization, with memory allocation
 \param[in] sze The capacity of the buffer
 \exception      If impossible to allocate memory
*/

Buffer::Buffer(u_int32_t sze) throw(DbException): size(0),capacity(sze)
{
    bfr = malloc(capacity);
    if (bfr==NULL)
        throw(BdbhException("not enough memory"));
}

/** 
    Free the memory
*/

Buffer::~Buffer()
{
    free(bfr);
}


/** Set the current capacity of the buffer

 - If the capacity (in bytes) passed by parameter is lower than the actual capacity, make nothing
 - If higher, realloc the buffer - WARNING, the base address has changed, so if you made a GetData() call, it is no more valid
  
 \param[in] c The capacity requested
 \exception If the wanted size is higher than the capacity
*/

void Buffer::SetCapacity(u_int32_t c) throw(BdbhException)
{
    if (c < 1024 || c < capacity)
        return;
    
    c /= 1024;      // 4345 ==> 5120
    c += 1;
    c = c * 1024;
    void* tmp = realloc(bfr,c);
    if (tmp == NULL)
        throw(BdbhException("Buffer: not enough memory"));
    
    bfr = tmp;
    capacity = c;
}

/** Set the current size of the buffer

 - If the size (in bytes) passed by parameter is lower than the actual capacity, only update private variable size
 - If higher, call realloc - WARNING THIS WILL CHANGE THE BASE ADDRESS, you mut call GetData()
 - If size==-1, the current capacity of the buffer is chosen
 - If size==0, the buffer size is 0, marking the buffer as empty
  
 \param[in] s The size requested
 \exception If the wanted size is higher than the capacity
*/

void Buffer::SetSize(int32_t s) throw(BdbhException)
{
    if (s<=-1)
    {
        size = capacity;
    }
    else
    {
        if ((u_int32_t)s > capacity)
        {
            ////throw(BdbhException("Buffer: you asked for a size higher than capacity."));
            SetCapacity(s);
        }
        size = (u_int32_t)s;
    }
}

    
/** Command initialization

\param p The parsed command-line parameters
\param d A BerkeleyDb object

*/
 
Command::Command(const Parameters   & p, BerkeleyDb& d): prm(p),exit_status(0),bdb(d),bfr3(__InitBfr3()) {}
bdbh::TriBuff* Command::bfr3_ptr = NULL;

/** BerkeleyDb initialization

\param name The file name
\param o_mode The opening mode
\param cluster If true, we lock the database before opening it (the --cluster switch was specified)
\param verbose If true, we are in verbose mode (the --verbose switch was specified)
Init the Dbt objects, open the database, read the info_data and create a cursor
*/
BerkeleyDb::BerkeleyDb(const char* name,int o_mode, bool verb, bool clus, bool inmem) throw(DbException): 
#if NOCLUSTER
#else
	cluster(clus),
	lock_file(-1),
	lock_inf_written(false),
	update_cnt(0),
	db_name(name),
#endif
	verbose(verb),
    in_memory(inmem),
	ignore_comp(false),
	db_env(NULL),
	db(NULL),
    pagesize(sysconf(_SC_PAGESIZE)),
#if NOCLUSTER
#else
	qinfo(NULL),
#endif
	open_mode(o_mode),
	data_bfr(1000),
	key_bfr(1000)
{
	open_mode = o_mode;
    // Init the Dbt objects
    dbt_key.set_flags(DB_DBT_USERMEM);

    dbt_mdata.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
    dbt_mdata.set_doff(0);
    dbt_mdata.set_dlen(sizeof(Mdata));
    dbt_mdata.set_ulen(sizeof(Mdata));
    dbt_mdata.set_size(sizeof(Mdata));

    dbt_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
    dbt_data.set_doff(sizeof(Mdata));

    // Init the metadata for info
    minfo_data.size = sizeof(InfoData);
    minfo_data.csize= sizeof(InfoData);
    
    // Open the database and read info_data
    // NOTE - __InitDb must be called AFTER dbt_* initialization, otherwise it would be impossible
    //        to read info_data
    __InitDb(name,o_mode);
    
}

/** Check alignment
 
 \brief Return true if p is aligned vs pagesize
 \note  stolen from vmtouch (see __InMemory)
 
 \param p An address
 \return true/false
 
*/
bool BerkeleyDb::__aligned_p(void *p) {
  return 0 == ((long)p & (pagesize-1));
}

/** Size conversion
 
 \brief Convert a size in bytes to a size in pages
 \note  stolen from vmtouch (see __InMemory)
 
 \param size A size in bytes
 \return size in pages
 
*/
uint32_t BerkeleyDb::__bytes2pages(uint64_t size) throw (BdbhException) {
    uint64_t szp = (size+pagesize-1) / pagesize;

// We could compile in c++11 to avoid this stupid stuff
#ifndef UINT32_MAX
#define UINT32_MAX  0xffffffff
#endif

    if (szp>UINT32_MAX) {
        ostringstream msg;
        msg << "ERROR - Size in pages should be < " << UINT32_MAX;
        throw(BdbhException(msg.str().c_str()));
    }
    return szp;
}

/** Test last bit
 
 \brief return true if last bit of p is 1
 */
bool BerkeleyDb::__is_mincore_page_resident(char p) {
  return p & 0x1;
}

/** Open the database

\brief Open the two databases: db (all modes) and qinfo (only in create or write modes) 
\param name The database name
\param flg A flag to pass to db->open
\exception db->open may throw some exception
*/
void BerkeleyDb::__InitDb(const char* name, int flg) throw(DbException)
{
    // Set the cache size to 64 Mbytes
    //DOES NOT WORK WITH THE ENVIRONMENTS
    //db->set_cachesize(0,1<<26,1);

    switch(flg)
    {
        // Create the environment, database and info
        case BDBH_OCREATE:
        {
            __OpenCreate(name);
            break;
        };
        
        // Read mode: try to open the environment or at least the "database" file
        case BDBH_OREAD:
        {
            __OpenRead(name,db,"database");

            // Checking the database
            __ReadInfoData();
            break;
        };

        // Try to open the environment and both databases, or throw an exception
        case BDBH_OWRITE:
        {
            __OpenWrite(name,db,"database");
#if NOCLUSTER
#else
            __OpenWrite(name,qinfo,"info");
#endif
            // Checking the database and initializing cons_info_data
            __ReadInfoData();
            break;
        }

        // Same as OWRITE, but consolidate info and do not reset info_data
        case BDBH_OSHELL:
        {
#if NOCLUSTER
            __OpenWrite(name,db,"database");
            __ReadInfoData();
#else
            __OpenWrite(name,db,"database");
            __OpenWrite(name,qinfo,"info");
            __ReadInfoData();
            __ConsolidateInfoData();
#endif
            break;
        }

        // Try to open the environment and database only, or throw an exception
        case BDBH_OCONVERT:
        {
            __OpenWrite(name,db,"database");
            break;
        }
        
        // Try to open the environment, database or info, but do not throw any exception
        case BDBH_OINFO:
        {
            try
            {
#if NOCLUSTER
                __OpenRead(name,db,"database");
                __ReadInfoData();
#else
                __OpenWrite(name,db,"database");
                __OpenWrite(name,qinfo,"info");
                __ReadInfoData();
                __ConsolidateInfoData();
#endif
            }
            catch(exception& e){
                __OpenRead(name,db,"database");
#if NOCLUSTER
#else
                qinfo = (Db_aptr)NULL;
#endif
                __ReadInfoData();
            };
            break;
        }
    }
}


/** Put the database in memory
\brief Read the file: database and keep it inside the cache

\param db_name The database file name

\note This code is inspired by vmtouch, by Doug Hoyte and contributors. cf. https://github.com/hoytech/vmtouch

*/ 
void BerkeleyDb::__InMemory(const string& db_name) throw(BdbhException) {

    string msg;
    
    // Try to open file, check the size, etc.
    int fd = open(db_name.c_str(), O_RDONLY, 0);
    if (fd==-1) {
        msg = "ERROR - Could not open file ";
        msg += db_name;
        msg += (string) " to put it in memory";
        throw(BdbhException(msg.c_str()));
    }
    
    struct stat db_name_stat;
    int rvl = fstat(fd, &db_name_stat);
    if (rvl) {
        msg = "ERROR - Could not stat file ";
        msg += db_name;
        throw(BdbhException(msg.c_str()));
    }
    
    if ( ! S_ISREG(db_name_stat.st_mode)) {
        msg = "ERROR - ";
        msg += db_name;
        msg += " should be a regular file, giving up";
        throw(BdbhException(msg.c_str()));
    }
        
    uint64_t size_in_bytes = db_name_stat.st_size;
    if (size_in_bytes == 0) return;
    
    if (size_in_bytes > (uint64_t) MAX_INMEMORY_SIZE) {
        ostringstream msg;
        msg << "ERROR - " << db_name << " is too large to be put in memory ";
        msg << "(size=" << size_in_bytes << " max=" << MAX_INMEMORY_SIZE << ")\n";
        throw(BdbhException(msg.str().c_str()));
    }

    // mmap the file to memory and check the alignment       
    void* mem = mmap(NULL, size_in_bytes, PROT_READ, MAP_SHARED, fd, 0);

    if (mem == MAP_FAILED) {
        msg = "ERROR - mmap returned an error (mmapped file ";
        msg += db_name;
        msg += ")";
        throw(BdbhException(msg.c_str()));
    }
    
    if (!__aligned_p(mem)) {
        ostringstream msg;
        msg << "ERROR - mmap was not page aligned: mem="<<mem<<" pagesize="<<pagesize;
        throw(BdbhException(msg.str().c_str()));
    }

    // Determining pages already in memory
    uint32_t size_in_pages = __bytes2pages(size_in_bytes);
    Buffer mincore_array_bfr(size_in_pages);
    unsigned char *mincore_array = (unsigned char*) mincore_array_bfr.GetData();
    uint32_t pages_in_core=0;

    // 3rd arg to mincore is char* on BSD and unsigned char* on linux
    if (mincore(mem, size_in_bytes, mincore_array)) throw (BdbhException("ERROR - mincore returned NULL"));

    for (uint32_t i=0; i<size_in_pages; i++) {
      if (__is_mincore_page_resident(mincore_array[i])) {
        pages_in_core++;
      }
    }

    // Read the whole file unless already in memory
    if (pages_in_core < size_in_pages ) {
        size_t junk_counter=0; // just to prevent any compiler optimizations
        for (size_t i=0; i<size_in_pages; i++) {
            junk_counter += ((char*)mem)[i*pagesize];
        }
    }
}

/** Create the environment and the databases

NOTE - We DO NOT USE THE BERKELEYDB ENVIRONMENT IF NOCLUSTER IS DEFINED

\param name The database name
\exception db->open may throw some exception
*/
#if NOCLUSTER
void BerkeleyDb::__OpenCreate(const char* name) throw(DbException)
{
    mkdir (name,0777);
	string name_db=name;
	name_db += '/';
	string name_database = name_db + "database";
	string name_info     = name_db + "info";
	db     = (Db_aptr) new Db(NULL,0);
    db->open(NULL,name_database.c_str(),NULL,
			 DB_BTREE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#if NOCLUSTER
#else
	qinfo  = (Db_aptr) new Db(NULL,0);
    qinfo->set_re_len(sizeof(Mdata)+sizeof(InfoData));
    qinfo->open(NULL,name_info.c_str(),NULL,
				DB_QUEUE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#endif
}
#else
void BerkeleyDb::__OpenCreate(const char* name) throw(DbException)
{
    mkdir (name,0777);
    db_env = (DbEnv_aptr) new DbEnv(0);
    db_env->open(name,DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL|DB_CDB_ALLDB,0);
    db     = (Db_aptr) new Db(db_env.get(),0);
    db->open(NULL,"database",NULL,DB_BTREE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    qinfo  = (Db_aptr) new Db(db_env.get(),0);
    qinfo->set_re_len(sizeof(Mdata)+sizeof(InfoData));
    qinfo->open(NULL,"info",NULL,DB_QUEUE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
}
#endif

/** Open the environment and one database for a read only access

\brief If name is a directory, try to open the environment (unless already opened), or at least the database_name file (read only).
If name is a file, try to open (read only) this file.

\param name The database name
\param database The database to open (database or info)
\param database_name The database file name
\exception db->open may throw some exception
*/
#if NOCLUSTER
void BerkeleyDb::__OpenRead(const char* name,Db_aptr& dbse, const char * database_name) throw(DbException)
{
    string db_name = name;
    stat(name,&bd_stat);       // Keep track of (device, inode)
    
    // If a directory, try to open the environment. If it succeeds, try to open database, if it fails try something else
    if (S_ISDIR(bd_stat.st_mode))
    {
		db_name += "/";
		db_name += database_name;
    }

    // in-memory treatments: read the database file in cache before any treatment 
    if (in_memory) __InMemory(db_name);

    dbse = (Db_aptr) new Db(0,0);
    dbse->open(NULL,db_name.c_str(),NULL,DB_UNKNOWN,DB_RDONLY,0);
}
#else
void BerkeleyDb::__OpenRead(const char* name,Db_aptr& dbse, const char * database_name) throw(DbException)
{
    stat(name,&bd_stat);       // Keep track of (device, inode)
    
    // If a directory, try to open the environment. If it succeeds, try to open database, if it fails try something else
    if (S_ISDIR(bd_stat.st_mode))
    {
        try
        {
            __LockRead(name);
        }
        catch(BdbhException& e)
        {
            string msg = e.what();
            msg += "\n";
            msg += "ERROR - May be created with bdbh 1.1 ? Please try the convert command";
            throw(BdbhException(msg.c_str()));
        }
        try
        {
            if (db_env.get()==NULL)
            {
                db_env = (DbEnv_aptr) new DbEnv(0);
                db_env->open(name,DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL|DB_CDB_ALLDB,0);
            }
            dbse = (Db_aptr) new Db(db_env.get(),0);
			dbse->set_error_stream(NULL);
            dbse->open(NULL,database_name,NULL,DB_UNKNOWN,DB_RDONLY,0);
        }
        // Couldn't open properly the database. Try to open just the database file (this makes sense in readonly)
        catch(DbException& e)
        {
            string db_name = name;
            db_name += "/";
            db_name += database_name;
            dbse = (Db_aptr) new Db(0,0);
            dbse->open(NULL,db_name.c_str(),NULL,DB_UNKNOWN,DB_RDONLY,0);
        }
    }
    // This is not a directory: just try to open as a file
    else
    {
        dbse = (Db_aptr) new Db(0,0);
        dbse->open(NULL,name,NULL,DB_UNKNOWN,DB_RDONLY,0);
    }
}
#endif

/** Open the environment and one database for a write access

\brief If name is a directory, try to open the environment (unless already opened) and the database

\param name The database name
\param database The database to open (database or info)
\param database_name The database file name
\exception db->open may throw some exception
*/
#if NOCLUSTER
void  BerkeleyDb::__OpenWrite(const char* name, Db_aptr& dbse, const char* database_name) throw(DbException,BdbhException)
{
    stat(name,&bd_stat);       // Keep track of (device, inode)
    if (S_ISDIR(bd_stat.st_mode))
    {
		string db_name = name;
		db_name += "/";
		db_name += database_name;
        
		dbse     = (Db_aptr) new Db(NULL,0);
        dbse->open(NULL,db_name.c_str(),NULL,DB_UNKNOWN,0,0);
    }
    else
    {
        throw(BdbhException("ERROR - May be created with bdbh 1.1 ? Please try the convert command"));
    }
}
#else
    try
    {
        __LockWrite(name);
    }
    catch(BdbhException& e)
    {
        string msg = e.what();
        msg += "\n";
        msg += "ERROR - May be created with bdbh 1.1, or permission problem";
        throw(BdbhException(msg.c_str()));
    }
    
    stat(name,&bd_stat);       // Keep track of (device, inode)
    if (S_ISDIR(bd_stat.st_mode))
    {
        if (db_env.get()==NULL)
        {
            db_env = (DbEnv_aptr) new DbEnv(0);
            db_env->open(name,DB_CREATE|DB_INIT_CDB|DB_INIT_MPOOL|DB_CDB_ALLDB,0);
        }
        dbse     = (Db_aptr) new Db(db_env.get(),0);
        dbse->open(NULL,database_name,NULL,DB_UNKNOWN,0,0);
    }
    else
    {
        throw(BdbhException("ERROR - May be created with bdbh 1.1 ? Please try the convert command"));
    }
}
#endif

#if NOCLUSTER
#else

/** Create a lock, when the lock will be granted nobody else will be enable to access (read or write) the database

\brief We lock (write lock) a file in the environment directory, initializing members lock_xxx

\param name The directory name
\exception a BdbhException is thrown if the lock cannot be put
*/

void BerkeleyDb::__LockWrite(const char* name) throw(BdbhException)
{
    if (!cluster)
        return;
    
    string lock_file_name = db_name + "/lock";
    string lock_inf_file_name = lock_file_name + ".inf";
    flock lck;
    lck.l_type   = F_WRLCK;
    lck.l_whence = SEEK_SET;
    lck.l_start  = 0;
    lck.l_len    = 0;
    
    // Open the lock file, using the low level C library (because of the lock)
    lock_file = open(lock_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRUSR|S_IWUSR);
    if (lock_file==-1)
    {
        ostringstream err;
        err << "Cannot open the lock file " << lock_file_name << " (error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }

    // Lock the file
    int rvl = fcntl(lock_file,F_SETLKW,&lck);
    if (rvl == -1)
    {
       ostringstream err;
        err << "Cannot lock the database (file " << lock_file_name << ", error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }
        
    // Open the lock info file
    int lock_inf_file = open(lock_inf_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRUSR|S_IWUSR);
    if (lock_inf_file==-1)
    {
        ostringstream err;
        err << "Cannot open the lock file " << lock_inf_file_name << " (error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }

    // get some info about the process and the host
    char* hst = (char*) malloc(100*sizeof(char));
    gethostname(hst,99);
    pid_t pid = getpid();
    ostringstream msg;
    msg << "this database is locked by the process " << pid << " running on the node " << hst << '\n';
    int len = msg.str().length();
   
    // write this info to the lock info file
    write(lock_inf_file,msg.str().c_str(),len);
    
    // close the lock_inf_file
    close(lock_inf_file);
    
    // Remember the lock
    lock_inf_written = true;
}

/** Create a lock, when the lock will be granted nobody else will be enable to write the database

\brief We lock (read lock) a file in the environment directory, initializing only the member lock_file

\param name The directory name
\exception a BdbhException is thrown if the lock cannot be put
*/

void  BerkeleyDb::__LockRead(const char* name) throw(BdbhException)
{
       if (!cluster)
           return;
    
    // using a local variable for lock_file_name
    string lock_file_name = db_name + "/lock";
    string lock_inf_file_name = lock_file_name + ".inf";

    flock lck;
    lck.l_type   = F_RDLCK;
    lck.l_whence = SEEK_SET;
    lck.l_start  = 0;
    lck.l_len    = 0;
    
    // Open the lock file, using the low level C library (because of the lock)
    // using the member lock_file (will be closed by destructor)
    lock_file = open(lock_file_name.c_str(),O_RDONLY);
    if (lock_file==-1)
    {
        ostringstream err;
        err << "Cannot open the lock file " << lock_file_name << " (error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }

    // Lock the file
    int rvl = fcntl(lock_file,F_SETLKW,&lck);
    if (rvl == -1)
    {
       ostringstream err;
        err << "Cannot lock the database (file " << lock_file_name << ", error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }
    
    // try to open the lock_inf file
    int lock_inf_file = open(lock_inf_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRUSR|S_IWUSR);
    if (lock_inf_file!=-1)
    {

        // get some info about the process and the host
        char* hst = (char*) malloc(100*sizeof(char));
        gethostname(hst,99);
        pid_t pid = getpid();
        ostringstream msg;
        msg << "this database is locked by the process " << pid << " running on the node " << hst << '\n';
        int len = msg.str().length();
        
        // write this info to the lock info file
        write(lock_inf_file,msg.str().c_str(),len);

        // close the lock_inf_file
        close(lock_inf_file);

    }
    
    // Remember the lock
    lock_inf_written = "true";    
}
#endif

/** Close the database

*/
#if NOCLUSTER
BerkeleyDb::~BerkeleyDb()
{
    Sync();

	// close db
	if (db.get() != NULL) db->close(0);
}
#else
 BerkeleyDb::~BerkeleyDb()
{
    // If the lock was used (see the switch --cluster), remove and close the lock_file, this will release the lock
    if (lock_inf_written)
    {
        string lock_inf_file_name = db_name + "/lock.inf";
        unlink (lock_inf_file_name.c_str());
    }
    if (lock_file != -1)
        close(lock_file);
    Sync();
}
#endif

/** Sync the database

    Write the info data in the database, then call sync
    NOTE db->sync generates errors when ran from valgrind
*/

void BerkeleyDb::Sync(bool with_consolidate_info)

{
	if (open_mode==BDBH_OWRITE || open_mode==BDBH_OSHELL)
		//if (open_mode!=BDBH_OREAD)
    {
        __WriteInfoData();
#if NOCLUSTER
#else
        if (with_consolidate_info)
            __ConsolidateInfoData();
#endif
    	db->sync(0);
    }
}

/** Create a directory in the database

   \param fkey
*/
void Command::_Mkdir(const Fkey& fkey)
{
    string key = fkey.GetKey();

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
    
    // It is a directory (may be a leaf, may be not)
    mdata.mode = S_IFDIR | 0770;    // It is a directory, rw-rw-rw (will be changed by umask if retrieved to a file)
    
    _Mkdir(fkey,mdata);
    
    // Create the special empty file to mark the start of directory list
    key += DIRECTORY_MARKER;
    mdata.mode = S_IFREG | 0666;
    GetDataBfr().SetSize(0);
    
    _WriteKeyData(key,mdata);
    __UpdateDbSize(0,0,key.size(),0,0);          // Do not count the file, only the key size

}

/** Write the directory to the database, WITHOUT any recursive call

  \brief Write the metadata only to the database (no data to write)
  \param fkey
  \param[in,out] mdata The metadata, may be changed
*/
void Command::_Mkdir(const Fkey& fkey, Mdata& mdata)
{
    string key = fkey.GetKey();
    
     if (!_IsInDb(key.c_str(),mdata))
	 {

		 // Alloc an empty buffer, so that only the metadata will be stored
		 GetDataBfr().SetSize(0);
		 _WriteKeyData(key,mdata);
		 __UpdateDbSize(0,0,key.size(),0,1);          // adding a directory to the count
		 
		 // Create the special empty file to mark the start of directory list
		 key += DIRECTORY_MARKER;
		 mdata.mode = S_IFREG | 0666;
		 GetDataBfr().SetSize(0);
		 
		 _WriteKeyData(key,mdata);
		 __UpdateDbSize(0,0,key.size(),0,0);          // Do not count the file, only the key size
		 
		 prm.Log("key " + key + " updated (directory)",cerr);
	 }
}

/** Write a (key,data) pair 

\pre The data to write must be deposited inside data_bfr
 
 \param skey The key 
 \param[in,out] metadata - The metadata, modified in compression mode
 \param[in,out] data_bfr - The data buffer
 \param[in,out] c_data_bfr A buffer used for compressed data (could be a local variable, but passed here for performance)
 \param[in] with_data If false, only the metadata are written
 \param[in] to_db If true (default), we write to db - If false, we write to qinfo
 \param[in] overwrite If true (default), it is OK to overwrite a key
 \return The value return by Db::put
 \exception db->put may throw exception, We throw also an exception if compress returns an error 

*/
int BerkeleyDb::WriteKeyData(const string& skey, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data, bool to_db, bool overwrite ) throw(BdbhException,DbException)
{

    const char* key = skey.c_str();
    
    // Prepare the dbt for the key
    size_t key_size = strlen(key)+sizeof(char)+1;      // With the level and the \0
    if (!to_db && key_size < 50)     // qinfo is a queue database, it will return a generated key, make enough size for it
        key_size = 10;
    
    dbt_key.set_ulen(key_size);		
    dbt_key.set_size(key_size);
    //dbt_key.set_data((void*)key);
    key_bfr.SetSize(key_size);
    
    void * base = key_bfr.GetData();
    char * k_base = (char*) base + sizeof(char);
    dbt_key.set_data(base);
    
    // copy the key to key_bfr, keeping 1 char in front of the key
    memcpy(k_base, (const void*) key, key_size-sizeof(char));
    *(char*)base = 'A' + CountLevel(key);
    
    // Prepare the dbt for the metadata
    dbt_mdata.set_data((void*)&metadata);

    // the following useful only if data must be stored
    if (with_data)
    {
        // Compress or not the data and prepare the dbt for the data
        if (GetCompressionFlag() && !ignore_comp && data_bfr.GetSize() >= COMPRESSION_THRESHOLD)
        {
            // Compress the data
            c_data_bfr.SetSize(data_bfr.GetSize()+data_bfr.GetSize()/10);
            uLongf dest_len = c_data_bfr.GetSize();
            int rvl=compress ((Bytef*)c_data_bfr.GetData(), &dest_len, (Bytef*)data_bfr.GetData(), data_bfr.GetSize());
            if (rvl != Z_OK)
            {
                ostringstream tmp;
                tmp << "Zlib error - compress returned " << rvl;
                throw (BdbhException(tmp.str()));
            };
            c_data_bfr.SetSize(dest_len);
            
            // Keep the compressed status and size in the metadata
            metadata.cmpred = true;
            metadata.csize = dest_len;
            
            // and prepare the dbt for the data
            dbt_data.set_dlen(dest_len);
            dbt_data.set_ulen(dest_len);
            dbt_data.set_size(dest_len);
            dbt_data.set_data(c_data_bfr.GetData());
        }
        else
        {
            // prepare the dbt for the data (uncompressed)
            dbt_data.set_dlen(data_bfr.GetSize());
            dbt_data.set_ulen(data_bfr.GetSize());
            dbt_data.set_size(data_bfr.GetSize());
            dbt_data.set_data(data_bfr.GetData());
        };
    };        

    // Store the (key,metadata) pair
    // We write to db
    int rvl=-1;
    if (to_db)
    {
        u_int32_t flag = overwrite ? 0 : DB_NOOVERWRITE;
        rvl = db->put (0, &dbt_key, &dbt_mdata, flag);
        if (rvl !=0)
            return rvl;
        
        // Store the (key,data) pair only if there are data to store
        if (with_data && data_bfr.GetSize() != 0)
        {
            rvl = db->put (0, &dbt_key, &dbt_data, 0);
            return rvl;
        }
    }
#if NOCLUSTER
#else    
    // We write to qinfo
    // First write = The key is given by the database (using the Append mode)
    else
    {
        rvl = qinfo->put (0, &dbt_key, &dbt_mdata, DB_APPEND);
        if (rvl !=0)
            return rvl;
        
        // Store the (returned key, data) pair if there are data to store
        if (with_data && data_bfr.GetSize() != 0)
        {
            rvl = qinfo->put(0, &dbt_key, &dbt_data, 0);
            return rvl;
        }
    }
#endif
    return rvl;
}

/** Read the metadata and the data from the database, using the key

 \post The data are deposited in data_bfr and metadata is set
 \param skey The key (a string) (NOT USED IF FROM_DB FALSE, because info is a QUEUE)
 \param[in,out] metadata - The metadata, modified in compression mode
 \param[in,out] data_bfr - The data buffer
 \param[in,out] c_data_bfr A buffer used for compressed data (could be a local variable, but passed here for performance)
 \param with_data If false, only the metadata are returned
 \return 0 if success, DB_NOTFOUND if key not found

 \exception  db->get may throw exception
 We throw also an exception if compress returns an error 
*/

int BerkeleyDb::ReadKeyData(const string& skey, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data) throw(DbException,BdbhException)
{

    const char* key = skey.c_str();
    
    // Prepare the dbt for the key    
    size_t key_size = strlen(key)+sizeof(char)+1;      // With the level and the \0
    dbt_key.set_ulen(key_size);		
    dbt_key.set_size(key_size);
    //dbt_key.set_data((void*)key);
    key_bfr.SetSize(key_size);
    
    void * base = key_bfr.GetData();
    char * k_base = (char*) base + sizeof(char);
    dbt_key.set_data(base);
    
    // copy the key to key_bfr, keeping 1 char in front of the key
    memcpy(k_base, (const void*) key, key_size-sizeof(char));
    *(char*)base = 'A' + CountLevel(key);

    // Prepare the dbt for the metadata
    dbt_mdata.set_data((void*) &metadata);
    
    // read the metadata
    int rvl = db->get(0,&dbt_key,&dbt_mdata,0);
    if (rvl == DB_NOTFOUND)
        return rvl;
    
    // If we do not need the data, it's finished
    if (!with_data)
        return rvl;
    
    // Data compressed: read data in c_data_bfr and uncompress them to data_bfr
    if (metadata.cmpred && !ignore_comp)
    {
        // Reading the compressed data to c_data_bfr
        // Adjust the size of the buffer
        c_data_bfr.SetSize(metadata.csize);

        // Prepare the dbt for the data
        dbt_data.set_dlen(c_data_bfr.GetSize());
        dbt_data.set_ulen(c_data_bfr.GetSize());
        dbt_data.set_data(c_data_bfr.GetData());

        // Read the data from the database
        rvl = db->get (0, &dbt_key, &dbt_data, 0);
        if (rvl == DB_NOTFOUND)     // Should not happen
            return rvl;
    
        // Uncompress the data
        data_bfr.SetSize(metadata.size);
        uLongf dest_len = data_bfr.GetSize();
        rvl = uncompress ((Bytef *) data_bfr.GetData(), &dest_len, (Bytef *) c_data_bfr.GetData(), c_data_bfr.GetSize());
        if (rvl != Z_OK)
        {
            ostringstream tmp;
            tmp << "Zlib error - compress returned " << rvl;
            throw (BdbhException(tmp.str()));
        };
        data_bfr.SetSize(dest_len);
    }
    
    // Data not compressed (or we do not want to uncompress data): read them to data_bfr
    else
    {
        // Adjust the size of the buffer - 
        // If data is compressed AND ignore_comp is set, we use the compressed size
        if (metadata.cmpred && ignore_comp)
            data_bfr.SetSize(metadata.csize);
        else
            data_bfr.SetSize(metadata.size);

        // Prepare the dbt for the data
        dbt_data.set_dlen(data_bfr.GetSize());
        dbt_data.set_ulen(data_bfr.GetSize());
        dbt_data.set_data(data_bfr.GetData());
    
        // Read the data from the database
        rvl = db->get (0, &dbt_key, &dbt_data, 0);
    };
    return rvl;
}

/** Read the metadata and/or the data from the database, using a cursor

    
  - If key=="", call cursor->get with the DB_FIRST/DB_NEXT flag, following the parameter first
    Select current_cursor as cursor[0]
    Return 0 if succcess, DB_NOTFOUND if no more data, or throw an exception if something else happens
  - If key=some/key: 
        if first==true:
              select cursor as cursors[CountLevel(key)]
              call cursor->get with the DB_SET flag
        if first==false:
              select cursor as cursors[lvl]
              call cursor->get with the DB_NEXT flag 
              return DB_NOTFOUND if the retrieved key does not match the BEGINNING of key

 \pre dbt_key, dbt_data, dbt_mdata must be correctly initialized
 \post The data are deposited in data_bfr and metadata is set
 \param skey The key (string)
 \param[out]  rtn_key The returned key (not necessarily the same as key), undefined if rvl != 0
 \param[in,out] metadata The metadata, modified in compression mode
 \param[in,out] key_bfr - The key buffer
 \param[in,out] data_bfr - The data buffer
 \param[in,out] c_data_bfr A buffer used for compressed data (could be a local variable, but passed here for performance)
 \param lvl The level of the key (not used if first==true)
 \param first True if first call, false if called in a loop
 \param with_data If false, only the metadata are returned
 \param reverse If reverse is true, first is changed to last, prev is changed to next [DOES NOT WORK, IS NOT USED YET, WILL BE REMOVED ?]
 \return 0 if success, DB_NOTFOUND if key not found
 \exception  db->get may throw exception We throw also an exception if compress returns an error 
*/
int BerkeleyDb::ReadKeyData(const string& skey, string& rtn_key, Mdata& metadata, Buffer& key_bfr, Buffer& data_bfr, Buffer& c_data_bfr, int lvl, bool first, bool with_data, bool reverse) throw(DbException,BdbhException)
{
    int rvl = 0;
    u_int32_t flags;
    
    const char* key = skey.c_str();   
    Dbc* cursor;
    
    if (strlen(key) == 0)
        flags = reverse ? DB_PREV:DB_NEXT;
    
    if (first)
    {
        if (strlen(key)==0)     // No key specified -> list everything 
            flags = reverse ? DB_LAST : DB_FIRST;
        else                    // Key specified -> list from this key
            flags = DB_SET;
        // The lvl parameter is ignored, the level is computed from the key instead
        lvl = CountLevel(key);
    }
    else
    {
        flags = reverse ? DB_PREV : DB_NEXT;
    };

    // Select the prefix character
    char prefix = 'A' + lvl;
    
    // Select the good cursor, using lvl
    cursor = __SelectCurrentCursor(lvl);

    // Prepare the dbt for the key    
    size_t key_size = strlen(key)+sizeof(char)+1;      // With the level and the \0

    key_bfr.SetSize(-1);   // 1 Mb, this should be enough for the key
    dbt_key.set_flags(DB_DBT_USERMEM);
    dbt_key.set_data(key_bfr.GetData());
    dbt_key.set_ulen(key_bfr.GetSize());
    dbt_mdata.set_data((void*) &metadata);

    if (first && strlen(key) != 0)
    {
        if (strlen(key) > key_bfr.GetSize())
            throw(BdbhException("Sorry, your key is too long"));
        
        void * base = key_bfr.GetData();
        char * k_base = (char*) base + sizeof(char);
        dbt_key.set_data(base);
        
        // copy the key to key_bfr, keeping 1 char in front of the key
        memcpy(k_base, (const void*) key, key_size-sizeof(char));
        *(char*)base = prefix;
        dbt_key.set_size(key_size);
    }
    else
    {
        dbt_key.set_size(0);
    };

    // Retrieve the metadata    
    rvl = cursor->get(&dbt_key,&dbt_mdata,flags);
    if (rvl == DB_NOTFOUND)
        return rvl;

    // In reverse mode AND if first: from this point, go to the last of the keys we want to retrieve
    if (first && reverse)
    {
        do
        {
            rvl = cursor->get(&dbt_key,&dbt_mdata,DB_NEXT);
        } while(rvl==0 && strstr((const char*)dbt_key.get_data(),key)==dbt_key.get_data());
        
        // If not at the end of db, we got too far away, but if we are at the end, the cursor is OK
        if (rvl ==0)
            rvl = cursor->get(&dbt_key,&dbt_mdata,DB_PREV);
        else
            rvl = 0;
    };
    
    // We retrieve the data as long as:
    //    - We are still at the same level (check the returned prefix)
    //    - the retrieved key has the same directories as key
    const char* rk = (const char*)dbt_key.get_data() + sizeof(char);
    if (!first)
    {
        // If we did not get the same level as skey, return
        if (*(char*)dbt_key.get_data() != prefix)
            return DB_NOTFOUND;

        // If skey is empty, we accept any key if same level
        if (skey.size()!=0)
        {
            // If both strings do not start the same, return
            if (strstr(rk,key)!=rk)
                return DB_NOTFOUND;
            /* We must detect the 2 situations:
                toto/titi
                toto/titi/tata OK, go on
                and       
                toto/titi
                toto/titi1 Not OK, return not found
            */
            else
            {
                if (rk[skey.size()] != '/')
                    return DB_NOTFOUND;
            }
        }
    }
    
    // OK - we can now return the key, and may be the data    
    rtn_key = (string) rk;

    // data to retrieve...
    if (with_data)
    {
        
        // Things to uncompress
        if (metadata.cmpred && !ignore_comp)
        {
            // Reading the compressed data to c_data_bfr
            // Adjust the size of the buffer
            c_data_bfr.SetSize(metadata.csize);
            
            // Prepare the dbt for the data
            dbt_data.set_dlen(c_data_bfr.GetSize());
            dbt_data.set_ulen(c_data_bfr.GetSize());
            dbt_data.set_data(c_data_bfr.GetData());
            
            // Read the data from the database
            rvl = cursor->get (&dbt_key, &dbt_data, DB_SET);
            if (rvl == DB_NOTFOUND)     // Should not happen
                return rvl;
            
            // Uncompress the data
            data_bfr.SetSize(metadata.size);
            uLongf dest_len = data_bfr.GetSize();
            rvl = uncompress ((Bytef *) data_bfr.GetData(), &dest_len, (Bytef *) c_data_bfr.GetData(), c_data_bfr.GetSize());
            if (rvl != Z_OK)
            {
                ostringstream tmp;
                tmp << "Zlib error - compress returned " << rvl;
                throw (BdbhException(tmp.str()));
            };
            data_bfr.SetSize(dest_len);
        }
        
        // Nothing to uncompress
        else
        {
            // Nothing to uncompress: reading the data to data_bfr
            // Adjust the size of the buffer - 
            // If data is compressed AND ignore_comp is set, we use the compressed size
            if (metadata.cmpred && ignore_comp)
                data_bfr.SetSize(metadata.csize);
            else
                data_bfr.SetSize(metadata.size);
            
            // Prepare the dbt for the data
            dbt_data.set_dlen(data_bfr.GetSize());
            dbt_data.set_ulen(data_bfr.GetSize());
            dbt_data.set_data(data_bfr.GetData());
            
            // Read the data from the database
            rvl = cursor->get (&dbt_key, &dbt_data, DB_SET);
        }
    }
    return rvl;
}
 
    
/** Select the current cursor, following the parameter

  Create the cursor if not yet created
  
\param lvl The level, also the index in cursors
\return The cursor selected
*/

Dbc* BerkeleyDb::__SelectCurrentCursor(unsigned int lvl)
{
    if (cursors.size()<=lvl)
    {
        for (unsigned int i=cursors.size(); i<=lvl; i++)
        {
            Dbc* cursor;
#if NOCLUSTER
			db->cursor(0,&cursor,0);
#else
            // Not sure the DB_WRITECURSOR flag is useful, however it works with an env specified			
            u_int32_t c_flg = (open_mode==BDBH_OWRITE||open_mode==BDBH_OSHELL ? DB_WRITECURSOR : 0);
            db->cursor(0,&cursor,c_flg);
#endif
            cursors.push_back(cursor);
        }
    }
    return cursors[lvl];
}

/** Throw an exception if the file is the db file
\param file_name A file name
\exception throw an exception if the file is the database file
*/
void BerkeleyDb::IsDbItself(const char* file_name) const throw(BdbhException)
{
    struct stat st;
    lstat(file_name, &st);
    IsDbItself(st);
}

/** Throw an exception if the file is the db file
\param st A return from stat ou fstat on some file
\exception throw an exception if the file is the database file
*/
void BerkeleyDb::IsDbItself(const struct stat& st) const throw(BdbhException)
{
    if (st.st_dev==bd_stat.st_dev && st.st_ino == bd_stat.st_ino)
        throw(BdbhException("Whouoh, This file represents myself. I can't support it."));
}

/** Write the time in a pretty way
*/
ostream& bdbh::operator<<(ostream& os,const Time& t)
{
    struct tm* tme = localtime(&t.tme);
    os << setw(4) << tme->tm_year+1900 << '-' << setw(2) << setfill('0') << tme->tm_mon+1 << '-' << setw(2) << tme->tm_mday << " ";
    os << setw(2) << tme->tm_hour << ':' << setw(2) << tme->tm_min << "  ";
    return os;
}

/** Read the info_data struct from the database
    Check to be sure we are reading the good version, and that it is the good architecture 
    The data are read from the database file
*/

void BerkeleyDb::__ReadInfoData() throw(DbException,BdbhException)
{
    // Read data - NOTE The compression buffer is not be used here
    int rvl = ReadKeyData(INFO_KEY,minfo_data, data_bfr, data_bfr, true);
    if (rvl==DB_NOTFOUND)
        throw(BdbhException("This is not a database created by bdbdh"));
    InfoData* inf_ptr = (InfoData*) data_bfr.GetData();
    //cerr << "__ReadInfoData " << inf_ptr->v_major << "\n";

    if (inf_ptr->v_major!=V_MAJOR || inf_ptr->v_minor>V_MINOR)
        throw(BdbhException("I cannot read this database (on which architecture did you create it ?)"));

#if NOCLUSTER
	info_data = *inf_ptr;
#else
    cons_info_data = *inf_ptr;
#endif

    // Consolidate info_data only in OINFO mode
    //if (open_mode==BDBH_OINFO && qinfo.get() != NULL)
    //    __ConsolidateInfoData();
}

#if NOCLUSTER
#else
/** InfoData::AddInfo

 - v_major, v_minor, date_created, data_compressed are never changed (and not checked)
 - max_key_size, max_data_size_uncompressed are updated if relevant
 - date_modified is updated
 - The other fields (may be < 0) are added

*/

InfoData& InfoData::AddInfo(const InfoData& i)
{
    if (i.max_data_size_uncompressed > max_data_size_uncompressed)
        max_data_size_uncompressed = i.max_data_size_uncompressed;
    if (i.max_key_size > max_key_size)
        max_key_size = i.max_key_size;

    data_size_uncompressed += i.data_size_uncompressed;
    data_size_compressed   += i.data_size_compressed;
    key_size    += i.key_size;
    nb_of_files += i.nb_of_files;
    nb_of_dir   += i.nb_of_dir;

    date_modified = i.date_modified;

    return *this;
}

/** Read every record from qinfo, deleting the record, add the read record to cons_info_data
    For each read record, update minfo_data
    Then, call __WriteInfodata, writing InfoData to database

*/
void BerkeleyDb::__ConsolidateInfoData() throw(DbException,BdbhException)
{
    // Check the info database to know the number of records (we cannot use DB_FAST_STAT, let's hope the database is not too huge)
    
    DB_QUEUE_STAT *stat_data=NULL;
    qinfo->stat(NULL, (void*) &stat_data, 0);
    u_int32_t ndata = stat_data->qs_ndata;
    //u_int32_t re_len= stat_data->qs_re_len;
    //u_int32_t first_recno= stat_data->qs_first_recno;
    free (stat_data);
    
    if (ndata==0)
        return;
 
	// TODO - Utiliser Parameters::Log ? Ou log4cxx ?
	if (verbose)
		cerr << "INFO - Completing information from the last added files: " << ndata << " record to add\n";
    //cerr << "hello, record_ length= " << re_len << '\n';
    //cerr << "hello, first rec nb  = " << first_recno << '\n';
    
    // Check to know if the environment was opened. If not, throw an exception
    try
    {
        u_int32_t flags;
        db_env->get_open_flags(&flags);
    }
    catch (DbException & e)
    {
        throw (BdbhException("ERROR - Info data cannot be consolidated - May be a permission problem ?"));
    }
    
    // If not empty, we have some data to read from qinfo
    // The allocated databuffer size is the reclen of qinfo
    data_bfr.SetSize(sizeof(Mdata)+sizeof(InfoData));
    
    // Prepare the dbt for the key
    key_bfr.SetSize(-1);   // 1 Mb, this should be enough for the key
    dbt_key.set_data(key_bfr.GetData());
    dbt_key.set_ulen(key_bfr.GetSize());
    
    // Prepare a new dbt for the data
    // NB This not too clean but we use a DB_CONSUME flag, so the parameters are NOT the same as for the "normal" db
    
    // Prepare the dbt for the data
    Dbt dbt_idata;
    dbt_idata.set_flags(DB_DBT_USERMEM);
    dbt_idata.set_dlen(data_bfr.GetSize());
    dbt_idata.set_ulen(data_bfr.GetSize());
    dbt_idata.set_size(data_bfr.GetSize());
    dbt_idata.set_data(data_bfr.GetData());
    dbt_idata.set_doff(0);
    
    // Access to the metadata/infodata from the buffer
    // WARNING - IS IT PORTABLE ? PAS SUR
    Mdata* tmp_minfo_data_ptr   = (Mdata*) data_bfr.GetData();
    InfoData* tmp_info_data_ptr = (InfoData*) (tmp_minfo_data_ptr+1);
    
    Mdata& tmp_minfo_data = *tmp_minfo_data_ptr;
    InfoData& tmp_info_data = *tmp_info_data_ptr;
    
    // We know the number of records to retrieve
    while(ndata-- > 0)
    {
        // Read the data from the database
        int rvl = qinfo->get (0, &dbt_key, &dbt_idata, DB_CONSUME);
        if (rvl != 0)
        {
            ostringstream tmp;
            tmp << "Bdb error, reading data from qinfo - qinfo->get returned " << rvl;
            throw (BdbhException(tmp.str()));
        }
        
        cons_info_data.AddInfo(tmp_info_data);
        // cerr << "coucou " << tmp_info_data.v_major << " " << info_data.nb_of_dir << "  " << info_data.nb_of_files << "\n";
    }
    
    // minfo_data is set to the LAST meta-info record read (only the date is useful)
    minfo_data = tmp_minfo_data;
    
    // Write the info_data to db
    __WriteInfoData(false);
    
    //Synchronize everything
    //db->sync(0);
    //qinfo->sync(0);
}
#endif

#if NOCLUSTER
/** Write the info_data struct to the database

	\brief the cons_info_data struct is written to db

*/
void BerkeleyDb::__WriteInfoData() throw(DbException)
{
	timeval tv;
	gettimeofday(&tv,NULL);

	// juste created !
	if (info_data.date_created==0) {
		info_data.date_created = tv.tv_sec;
	}
	info_data.date_modified = tv.tv_sec;
	data_bfr.SetSize(sizeof(InfoData));
        
	//InfoData* inf_ptr = to_qinfo ? &info_data : &cons_info_data;
	InfoData* inf_ptr = &info_data;
	memcpy(data_bfr.GetData(),(const void*) inf_ptr, sizeof(InfoData));
	WriteKeyData(INFO_KEY,minfo_data,data_bfr,data_bfr,true,true,true);
}
#else
/** Write the info_data struct to the database

\param to_qinfo If true (default), the info_data struct is enqueued to qinfo, then reset
If false, the cons_info_data struct is written to db

*/
void BerkeleyDb::__WriteInfoData(bool to_qinfo) throw(DbException)
{
	timeval tv;
	gettimeofday(&tv,NULL);
	info_data.date_modified = tv.tv_sec;
	data_bfr.SetSize(sizeof(InfoData));
        
	InfoData* inf_ptr = to_qinfo ? &info_data : &cons_info_data;
	memcpy(data_bfr.GetData(),(const void*) inf_ptr, sizeof(InfoData));
	WriteKeyData(INFO_KEY,minfo_data,data_bfr,data_bfr,true,!to_qinfo,true);
	if (to_qinfo)
	{
		// Resetting info_data
		info_data.Reset();
	}
}
#endif

/** Adjust the databuffers capacity 
*/
void Command::_AdjustBufferCapacity() throw(BdbhException)
{
    GetDataBfr().SetCapacity(bdb.GetMaxDataSize());
    GetCDataBfr().SetCapacity(bdb.GetMaxDataSize()+bdb.GetMaxDataSize()/10);
}

vector<string> Command::_ExpandWildcard(const string& k) throw(DbException,BdbhException)
{
    const string wc = "/*/";

    vector<string> expanded;
    vector<string> expanded2;

    // head is the part of k at left of *, tail is at right
    string head,tail;

    bool wchr_found = false;
    
    string::size_type w;

    // if found /*/
    if ((w=k.find(wc))!=string::npos)
    {
        head = k.substr(0,w);
        tail = k.substr(w+2);
        wchr_found = true;
    }
    
    // if found '*' (only character)
    else if (k[0]=='*'&&k.size()==1)
    {
        head = "";
        tail = "";
        wchr_found = true;
    }
    
    // if found */ (at start)
    else if (k[0]=='*'&&k[1]=='/')
    {
        head = "";
        tail = k.substr(1);
        wchr_found = true;
    }
    
    // if found /* (at end)
    else if(k.size()>=2&&k[k.size()-2]=='/'&&k[k.size()-1]=='*')
    {
        head = k.substr(0,k.size()-2);
        tail = "";
        wchr_found = true;
    }
    
    if (wchr_found)
    {
        string marker;
        if (head == "")
        {
            const char* m = DIRECTORY_MARKER;
            marker = m+1;   // skip the leading / which is at beginning of DIRECTORY MARKER
        }
        else
            marker = head + DIRECTORY_MARKER;
        string nkey;
        Mdata mdata;
        int lvl;
        if (head == "")
            lvl = 0;
        else
            lvl = CountLevel(head.c_str())+1;

        int rvl=_ReadKeyDataCursor(marker,nkey,mdata,lvl,true);
        if (rvl==DB_NOTFOUND)
        {
            string msg = "Problem in the database: the special file #" + marker +"# does not exsit"; 
            throw(BdbhException(msg));
        }
        while(rvl!=DB_NOTFOUND)
        {
            rvl=_ReadKeyDataCursor(head,nkey,mdata,lvl,false);
            if (rvl != DB_NOTFOUND)
                expanded.push_back(nkey+tail);
        }
        
        // Try a second pass (may be, there are several wildchars)
        for (unsigned int i=0; i<expanded.size(); i++)
        {
            string s = expanded[i];
            
            // If there is still at least 1 wildcard, expand s
            if (s.find('*') != string::npos)
            {
                vector<string> tmp = _ExpandWildcard(s);
                for (unsigned int j=0; j<tmp.size();j++)
                    expanded2.push_back(tmp[j]);
            }
            
            // If not, just push s
            else
            {
                expanded2.push_back(s);
            }
        }
    }
    
    // no wildcard found: return a vector with only one element: the parameter
    else
    {
        expanded2.push_back(k);
    }
    /*
    cout << "=== " << k << " ";
    for (int i=0; i<expanded2.size();i++)
        cout << expanded2[i] << " " ;
    cout << "====\n";
    */
    return expanded2;
}

