
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

Buffer::Buffer(u_int32_t sze): size(0),capacity(sze)
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

void Buffer::SetCapacity(u_int32_t c)
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

void Buffer::SetSize(int32_t s)
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
 
Command::Command(const Parameters   & p, BerkeleyDb& d): exit_status(0),prm(p),bdb(d),bfr3(__InitBfr3()) {}
bdbh::TriBuff* Command::bfr3_ptr = NULL;

/** BerkeleyDb initialization

\param name The file name
\param o_mode The opening mode
\param verbose If true, we are in verbose mode (the --verbose switch was specified)
\param inmem If true, read the whole datafile, getting the file in the I/O cache memory
Init the Dbt objects, open the database, read the info_data and create a cursor
*/
BerkeleyDb::BerkeleyDb(const char* name,int o_mode, bool verb, bool inmem):
	verbose(verb),
    in_memory(inmem),
	ignore_comp(false),
    dbi(NULL),
	db(NULL),
    pagesize(sysconf(_SC_PAGESIZE)),
	open_mode(o_mode),
	data_bfr(1000),
	key_bfr(1000)
{
	open_mode = o_mode;
    
    // Init the Dbt objects
    // ... for the keys
    dbt_key.set_flags(DB_DBT_USERMEM);

    // ... for the metadata
    dbt_mdata.set_flags(DB_DBT_USERMEM);
    dbt_mdata.set_doff(0);
    dbt_mdata.set_dlen(sizeof(Mdata));
    dbt_mdata.set_ulen(sizeof(Mdata));
    dbt_mdata.set_size(sizeof(Mdata));

    // ... for the inodes
    dbt_inode.set_flags(DB_DBT_USERMEM);
    dbt_inode.set_doff(0);
    dbt_inode.set_dlen(sizeof(bdbh_ino_t));
    dbt_inode.set_ulen(sizeof(bdbh_ino_t));
    dbt_inode.set_size(sizeof(bdbh_ino_t));
    dbt_inode.set_data((void*)&c_inode);

    // ... for the data
    // dbt_data.set_flags(DB_DBT_USERMEM|DB_DBT_PARTIAL);
    // dbt_data.set_doff(sizeof(Mdata));
    dbt_data.set_flags(DB_DBT_USERMEM);

    // Init the data for info
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
uint32_t BerkeleyDb::__bytes2pages(uint64_t size) {
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

\brief Open the db database
\param name The database name
\param flg A flag to pass to db->open
\exception db->open may throw some exception
*/
void BerkeleyDb::__InitDb(const char* name, int flg)
{

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
            __OpenRead(name);

            // Checking the database
            __ReadInfoData();
            break;
        };

        // Try to open the environment and both databases, or throw an exception
        case BDBH_OWRITE:
        {
            __OpenWrite(name);
            // Checking the database and initializing cons_info_data
            __ReadInfoData();
            break;
        }

        // Same as OWRITE, but consolidate info and do not reset info_data
        case BDBH_OSHELL:
        {
            __OpenWrite(name);
            __ReadInfoData();
            break;
        }

        // Try to open the environment and database only, or throw an exception
        case BDBH_OCONVERT:
        {
            __OpenWrite(name);
            break;
        }
        
        // Try to open the environment, database or info, but do not throw any exception
        case BDBH_OINFO:
        {
            try
            {
                __OpenRead(name);
                __ReadInfoData();
            }
            catch(exception& e){
                __OpenRead(name);
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
void BerkeleyDb::__InMemory(const string& db_name) {

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

/** Create the databases

NOTE - We DO NOT USE THE BERKELEYDB ENVIRONMENT

\param name The database name
\exception db->open may throw some exception
*/
void BerkeleyDb::__OpenCreate(const char* name)
{
    mkdir (name,0777);
	string name_db=name;
	name_db += '/';

    // The inodes
    string name_inodes   = name_db + METADATA_NAME;
	dbi    = (Db_aptr) new Db(NULL,0);
    dbi->open(NULL,name_inodes.c_str(),NULL,
			 DB_BTREE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    
    // The data
	string name_database = name_db + DATA_NAME;
	db     = (Db_aptr) new Db(NULL,0);
    db->open(NULL,name_database.c_str(),NULL,
			 DB_BTREE,DB_CREATE|DB_EXCL,S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
}

/** Open the database for a read only access

\brief Open the two databases (for the data and for the inodes)

\param name The database (=directory) name
\exception db->open may throw some exception
*/
void BerkeleyDb::__OpenRead(const char* name)
{
    __Open(name,DB_RDONLY);
}

/** Open the database for a write access

\brief Open the two databases (for the data and for inodes) 

\param name The database name
\exception db->open may throw some exception
*/
void BerkeleyDb::__OpenWrite(const char* name)
{
    __Open(name,0);
}

/** Open the database

\brief Called by __OpenRead and __OpenWrite, does the job

\param name The database name
\param flag The flag (read or write)
\exception db->open may throw some exception
*/
void BerkeleyDb::__Open(const char* name, u_int32_t flag)
{
    string db_name = name;
    stat(name,&bd_stat);       // Keep track of (device, inode)

    if (!S_ISDIR(bd_stat.st_mode))
    {
        string msg = db_name;
        msg += " should be a directory.\n";
        throw(BdbhException(msg));
    }
        
    db_name += "/";
    db_name += DATA_NAME;

    db = (Db_aptr) new Db(0,0);
    db->open(NULL,db_name.c_str(),NULL,DB_UNKNOWN,flag,0);

    string dbi_name = name;
    dbi_name += "/";
    dbi_name += METADATA_NAME;
    
    // in-memory treatments: read the database file in cache before any treatment 
    if (in_memory) __InMemory(dbi_name);

    dbi = (Db_aptr) new Db(0,0);
    dbi->open(NULL,dbi_name.c_str(),NULL,DB_UNKNOWN,flag,0);
}

/*
void  BerkeleyDb::__OpenWrite(const char* name)
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
*/

/** Close the database

*/
BerkeleyDb::~BerkeleyDb()
{
    Sync();

	// close db
	if (db.get() != NULL) db->close(0);
}

/** Sync the database

    Write the info data in the database, then call sync
    NOTE db->sync generates errors when ran from valgrind
*/

void BerkeleyDb::Sync(bool with_consolidate_info)

{
	if (open_mode==BDBH_OWRITE || open_mode==BDBH_OSHELL || open_mode==BDBH_CREATE)
		//if (open_mode!=BDBH_OREAD)
    {
        __WriteInfoData();
    	db->sync(0);
        dbi->sync(0);
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
    _UpdateDbSize(0,0,key.size(),0,0,0);          // Do not count the file, only the key size

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
         mdata.ino = _NextInode();
		 _WriteKeyData(key,mdata,false);
		 _UpdateDbSize(0,0,key.size(),0,1,0);          // adding a directory to the count
		 
		 // Create the special empty file to mark the start of directory list
		 key += DIRECTORY_MARKER;
		 mdata.mode = S_IFREG | 0666;
		 GetDataBfr().SetSize(0);
		 
         // Write the key-data
		 _WriteKeyData(key,mdata,false);
		 _UpdateDbSize(0,0,key.size(),0,0,0);
		 
		 prm.Log("key " + key + " updated (directory)",cerr);
	 }
}

/**
 * \brief From the file path, build the dbt_key
 *        The key is formed by a maj letter (A,B, ...) to code the level (number de directories), followed by the path
 * 
 * \param The path
 * 
 *****/
 void BerkeleyDb::__path2dbt_key(const string & path) {
    const char* key = path.c_str();
    
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
}

     
/** 
 * 
 \brief Write a (key,data) pair to the two databases db (data) and dbi (metadata)
 \pre The data to write must be deposited inside data_bfr
 
 \param path The path (ie the key)
 \param[in,out] metadata - The metadata, modified in compression mode and for inode initialization
 \param[in,out] data_bfr - The data buffer
 \param[in,out] c_data_bfr A buffer used for compressed data (could be a local variable, but passed here for performance)
 \param[in] with_data If false, only the metadata are written
 \param[in] to_db NOT USED !!!
 \param[in] overwrite If true (default), it is OK to overwrite a key
 \return The value return by Db::put
 \exception db->put may throw exception, We throw also an exception if compress returns an error 

*/
int BerkeleyDb::WriteKeyData(const string& path, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data, bool to_db, bool overwrite )
{

    // Prepare the dbt for the key
    __path2dbt_key(path);

    // Prepare the dbt for the metadata
    dbt_mdata.set_data((void*)&metadata);

    // the following useful only if data must be stored
    if (with_data)
    {
        // Copy the inode to c_inode, the dbt_inode points to it
        c_inode = metadata.ino;
        
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

    // Store the (inode,data) pair only if there are data to store
    // We start with the data because it is not so serious having a problem storing metadata than data
    int rvl=0;
    u_int32_t flag = overwrite ? 0 : DB_NOOVERWRITE;
    if (with_data && data_bfr.GetSize() != 0)
    {
        rvl = db->put (0, &dbt_inode, &dbt_data, flag);
    }

    // If all is OK, Store the (key,metadata) pair
    if (rvl==0) {
        rvl = dbi->put (0, &dbt_key, &dbt_mdata, flag);
    }
//    if (rvl !=0)
//        return rvl;

//    rvl = db->put (0, &dbt_key, &dbt_mdata, flag);
//    if (rvl !=0)
//        return rvl;
        
    
    return rvl;
}

/*****
 * @brief Remove metadata and data using a cursor
 * @pre The cursor selected by lvl must be positioned on the metadata to remove
 * @pre If the metadata is passed, remove also the data
 * 
 * @param The level, for selecting the good metadata cursor
 * @param mdata A ptr to the metadata, if nullptr data are not removed
 * @return 0 is OK
 * 
 *************************/
int BerkeleyDb::RemoveKeyDataCursor(int lvl, Mdata* metadata_ptr) {
    // Delete the data, unless metadata_ptr not specified
    if (metadata_ptr != nullptr) {
        // Directories do not have data
        if (!S_ISDIR(metadata_ptr->mode)) {
            c_inode = metadata_ptr->ino;
            int rvl = db->del(NULL, &dbt_inode, 0);
            if (rvl != 0 ) {
                throw BdbhException("INTERNAL ERROR");
            }
        }
    }
        
    // Delete the metadata and return
    return __SelectCurrentCursor(lvl)->del(0);
}   

/** Read the metadata and the data from the database, using the key

 \post The data are deposited in data_bfr and metadata is set
 \param path a path (ie a key, a string)
 \param[in,out] metadata - The metadata, modified in compression mode
 \param[in,out] data_bfr - The data buffer
 \param[in,out] c_data_bfr A buffer used for compressed data (could be a local variable, but passed here for performance)
 \param with_data If false, only the metadata are returned
 \return 0 if success, DB_NOTFOUND if key not found

 \exception  db->get may throw exception
 We throw also an exception if compress returns an error 
*/

int BerkeleyDb::ReadKeyData(const string& path, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data)
{

    // Prepare the dbt for the key
    __path2dbt_key(path);
    
    // Prepare the dbt for the metadata
    dbt_mdata.set_data((void*) &metadata);
    
    // read the metadata
    int rvl = dbi->get(0,&dbt_key,&dbt_mdata,0);
    if (rvl == DB_NOTFOUND)
        return rvl;
    
    // If we do not need the data, it's finished
    if (!with_data)
        return rvl;

    // Store the retrieved inode to c_inode, it will be used by the get operations
    c_inode = metadata.ino;
    
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
        rvl = db->get (0, &dbt_inode, &dbt_data, 0);
        //rvl = db->get (0, &dbt_key, &dbt_data, 0);
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
        rvl = db->get (0, &dbt_inode, &dbt_data, 0);
        // rvl = db->get (0, &dbt_key, &dbt_data, 0);
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

int BerkeleyDb::ReadKeyData(const string& skey, string& rtn_key, Mdata& metadata, Buffer& key_bfr, Buffer& data_bfr, Buffer& c_data_bfr, int lvl, bool first, bool with_data, bool reverse)
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
        // Copy the retrieved inode to c_inode
        c_inode = metadata.ino;
        
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
            ////rvl = cursor->get (&dbt_key, &dbt_data, DB_SET);
            ////db->get (0, &dbt_key, &dbt_data, 0);
            db->get (0, &dbt_inode, &dbt_data, 0);
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
            ////rvl = cursor->get (&dbt_key, &dbt_data, DB_SET);
            //db->get (0, &dbt_key, &dbt_data, 0);        }
            db->get (0, &dbt_inode, &dbt_data, 0);
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
			dbi->cursor(0,&cursor,0);
            cursors.push_back(cursor);
        }
    }
    return cursors[lvl];
}

/** Throw an exception if the file is the db file
\param file_name A file name
\exception throw an exception if the file is the database file
*/
void BerkeleyDb::IsDbItself(const char* file_name) const
{
    struct stat st;
    lstat(file_name, &st);
    IsDbItself(st);
}

/** Throw an exception if the file is the db file
\param st A return from stat ou fstat on some file
\exception throw an exception if the file is the database file
*/
void BerkeleyDb::IsDbItself(const struct stat& st) const
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

void BerkeleyDb::__ReadInfoData()
{
    // Read data - NOTE The compression buffer is not be used here
    int rvl = ReadKeyData(INFO_KEY,minfo_data, data_bfr, data_bfr, true);
    if (rvl==DB_NOTFOUND)
        throw(BdbhException("This is not a database created by bdbh"));
    InfoData* inf_ptr = (InfoData*) data_bfr.GetData();
    //cerr << "__ReadInfoData " << inf_ptr->v_major << "\n";

    if (inf_ptr->v_major!=V_MAJOR || inf_ptr->v_minor>V_MINOR)
        throw(BdbhException("I cannot read this database (on which architecture did you create it ?)"));

	info_data = *inf_ptr;

    // Consolidate info_data only in OINFO mode
    //if (open_mode==BDBH_OINFO && qinfo.get() != NULL)
    //    __ConsolidateInfoData();
}

/** Write the info_data struct to the database

	\brief the cons_info_data struct is written to db

*/
void BerkeleyDb::__WriteInfoData()
{
	timeval tv;
	gettimeofday(&tv,NULL);

	// juste created !
	if (info_data.date_created==0) {
		info_data.date_created = tv.tv_sec;
	}
	info_data.date_modified = tv.tv_sec;
	data_bfr.SetSize(sizeof(InfoData));
        
	InfoData* inf_ptr = &info_data;
	memcpy(data_bfr.GetData(),(const void*) inf_ptr, sizeof(InfoData));
	WriteKeyData(INFO_KEY,minfo_data,data_bfr,data_bfr,true,true,true);
}

/** Adjust the databuffers capacity 
*/
void Command::_AdjustBufferCapacity()
{
    GetDataBfr().SetCapacity(bdb.GetMaxDataSize());
    GetCDataBfr().SetCapacity(bdb.GetMaxDataSize()+bdb.GetMaxDataSize()/10);
}

vector<string> Command::_ExpandWildcard(const string& k)
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

