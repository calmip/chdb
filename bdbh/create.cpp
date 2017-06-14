
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#include "parameters.hpp"
#include "command.hpp"
#include "exception.hpp"
#include "create.hpp"

/** Create the database and write the info_data record
*/
void bdbh::Create::Exec() throw(BdbhException,DbException)
{
    // Prepare the metadata
    
    Mdata mdata;
    mdata.mode = S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;    // It is a regular file, mode rw-rw-rw-
    mdata.uid = getuid();
    mdata.gid = getgid();
    mdata.size=sizeof(InfoData);
    
    // time information = get the current time
    timeval tv;
    gettimeofday(&tv,NULL);
    mdata.atime = tv.tv_sec;
    mdata.mtime = tv.tv_sec;

    // build the InfoData structure
    InfoData info(prm.GetCompress());
    info.date_created = tv.tv_sec;
    info.date_modified = tv.tv_sec;
    
    // write data and metadata to the db
    GetDataBfr().SetSize((int32_t) sizeof(InfoData));
    memcpy(GetDataBfr().GetData(),(const void*) &info, sizeof(InfoData));
    _WriteKeyData(INFO_KEY,mdata);

#if NOCLUSTER
#else
    // Create the lock file
    string lock_file_name = prm.GetDatabase() + "/lock";
    int lock_file = open(lock_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRUSR|S_IWUSR);
    if (lock_file==-1)
    {
        ostringstream err;
        err << "Cannot open the lock file " << lock_file_name << " (error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }
    close(lock_file);
#endif

    prm.Log("Database " + prm.GetDatabase() + " created",cerr);
}

/** write back the info_data record with the correct release
*/
void bdbh::Convert::Exec() throw(BdbhException,DbException)
{
    // Read the INFO record
    Mdata mdata;
    int rvl = _ReadKeyData(INFO_KEY,mdata, true);
    if (rvl==DB_NOTFOUND)
        throw(BdbhException("ERROR - Convert::Exec, internal error"));
    
    InfoData* inf_ptr = (InfoData*) GetDataBfr().GetData();
    InfoData info = *inf_ptr;
    
    // time information = get the current time
    timeval tv;
    gettimeofday(&tv,NULL);
    mdata.mtime = tv.tv_sec;

    // Modify the InfoData structure
    info.date_modified = tv.tv_sec;
    info.v_minor = V_MINOR;
    
    // write data and metadata to the db
    GetDataBfr().SetSize((int32_t) sizeof(InfoData));
    memcpy(GetDataBfr().GetData(),(const void*) &info, sizeof(InfoData));
    _WriteKeyData(INFO_KEY,mdata);

    // Create the lock file
    string lock_file_name = prm.GetDatabase() + "/lock";
    int lock_file = open(lock_file_name.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRUSR|S_IWUSR);
    if (lock_file==-1)
    {
        ostringstream err;
        err << "Cannot open the lock file " << lock_file_name << " (error " << errno << ")";
        throw(BdbhException(err.str().c_str()));
    }
    close(lock_file);

    // Final message 
    string msg = " converted to release ";
    msg += '0'+V_MAJOR;
    msg += '.';
    msg += '0'+V_MINOR;
    prm.Log("Database " + prm.GetDatabase() + msg ,cerr);
}

/** Print the info_data record
*/
void bdbh::Info::Exec() throw(BdbhException,DbException)
{
	//_Sync();
    info_data = _GetInfoData();
	data_ready= true;
	if (no_print==false) {
		cout.setf(ios::left,ios::adjustfield);  
		cout <<  "database format version          " << info_data.v_major << '.' << info_data.v_minor << '\n';
		cout.setf(ios::right);
		cout << setw(33) << setfill(' ') << "creation date " << Time(info_data.date_created) << "\n";
		cout << setw(33) << setfill(' ') << "last modified " << Time(info_data.date_modified) << "\n";
		cout << setw(33) << setfill(' ') << "number of links+files " << info_data.nb_of_files << "\n";
		cout << setw(33) << setfill(' ') << "number of directories " << info_data.nb_of_dir << "\n";
		if (info_data.data_compressed)
		{
			cout << setw(33) << setfill(' ') <<  "compression " << "DATA COMPRESSED\n";
			cout << setw(33) << setfill(' ') <<  "data size (uncompressed) " << info_data.data_size_uncompressed << "\n";
			cout << setw(33) << setfill(' ') <<  "data size (compressed) " << info_data.data_size_compressed << "\n";
			cout << setw(33) << setfill(' ') <<  "largest data size (uncompressed) " << info_data.max_data_size_uncompressed << "\n";
		}
		else
		{
			cout << setw(33) << setfill(' ') <<  "compression " << "no\n";
			cout << setw(33) << setfill(' ') <<  "data size " << info_data.data_size_uncompressed << "\n";
			cout << setw(33) << setfill(' ') <<  "largest data size " << info_data.max_data_size_uncompressed << "\n";
		}
		cout << setw(33) << setfill(' ') <<  "key size " << info_data.key_size << "\n";
		cout << setw(33) << setfill(' ') <<  "largest key size " << info_data.max_key_size << "\n";
		//cout << setw(33) << setfill(' ') <<  "metadata size " << sizeof(Mdata) << "\n";
	}
}

/** 
	\brief Call Info::Exec, easier to use than the Info class

	\param bdb: An already opened BerkeleyDb
*/

bdbh::InfoData bdbh::GetInfo(bdbh::BerkeleyDb& bdb) {
	bdbh::Parameters prms;
	Info info(prms,bdb);
	bool saved_no_print = info.no_print;
	info.InhibitPrinting();
	info.Exec();
	info.no_print = saved_no_print;
	return info.GetInfoData();
}
bdbh::InfoData bdbh::GetInfo(const string& database) {
	bdbh::BerkeleyDb bdb(database.c_str(),BDBH_OREAD);
	return bdbh::GetInfo(bdb);
}

/** Print the usage of this program
*/
void bdbh::Help::Exec() throw(BdbhException,DbException)
{
    Usage();
}

void bdbh::Usage()
{
    cout << "bdbh [--database file.db] <switches> [command] [file or key] [...] \n";
	cout << "NOTE - A key is a path of a file or directory stored in the database\n";
    cout << "The command may be: create/info/add/extract/put/mkdir/cat/ls/rm/chmod/shell[/sync/q]\n";
    cout << "Default command: shell (go to interactive mode)\n";
    cout << "See also the environment variable BDBH_DATABASE\n";
   
    cout << "\nCOMMAND: create <--compress>\n";
    cout << "Create a new database\n";
    
    cout << "\nCOMMAND: info\n";
    cout << "Used to extract general information about the database\n";
    
    cout << "\nCOMMAND: add\n";
    cout << "Several files, directories or links may be specified\n";
    cout << "The symbolic links are not followed\n";
    cout << "See also the switches: --root, --recursive, --overwrite\n";
    cout << "See also the environment variable BDBH_ROOTA\n";
	cout << "WARNING - You CANNOT add a file bigger than " <<  MAX_FILE_SIZE/1024 << "kb\n";
    
    cout << "\nCOMMAND: extract\n";
    cout << "Several keys may be specified\n";
    cout << "The ownerships (if root), the modes etc. are recovered also\n";
    cout << "If you do not specify any parameter, THE WHOLE DATABASE WILL BE EXTRACTED\n";
    cout << "You may specify * as a wildcard directory name (ie toto/*/titi will be expanded, but NOT toto*/titi)\n";
    cout << "See also the switches: --root, --recursive, --level, --overwrite\n";
    cout << "See also the environment variable BDBH_ROOTX\n";
    
    cout << "\nCOMMAND: put\n";
    cout << "Only ONE key must be specified\n";
    cout << "Data are read from stdin or from --value and stored in the database\n";
    cout << "See also the switches: --root, --overwrite\n";
	cout << "WARNING - You CANNOT put more than " <<  MAX_FILE_SIZE/1024 << "kb, the data will be truncated if this limit is exceeded\n";

    cout << "\nCOMMAND: mkdir\n";
    cout << "Make directory in the database\n";
    cout << "See also the switches: --root\n";

    cout << "\nCOMMAND: cat\n";
    cout << "Only ONE key must be specified\n";
    cout << "The corresponding data are read from the database and sent to stdout\n";
    cout << "See also the switches: --root\n";
    
    cout << "\nCOMMAND: ls\n";
    cout << "Several keys may be specified\n";
    cout << "See also the switches: --long, --recursive, --size_sort, --reverse_sort, --level\n";

    cout << "\nCOMMAND: rm\n";
    cout << "Several keys may be specified\n";
    cout << "See also the switch: --recursive\n";
    
    cout << "\nCOMMAND: chmod\n";
    cout << "chmod of one or several keys - the mode can be specified as d755 (directories only) or f644 (files) or 0 (both)\n";
    cout << "See also the switches: --mode, --recursive, --level\n";

	cout << "\nCOMMAND: merge\n";
	cout << "merge the specified database with the current database\n";
    cout << "See also the switch: --verbose\n";
		
    cout << "\nCOMMAND: shell\n";
    cout << "Go to shell mode, where you may enter several commands, then leave the program\n";
    cout << "See also the switch: --stamp\n";

    cout << "\nCOMMAND: sync\n";
    cout << "IN shell MODE ONLY: Synchronize the database, flushing the last data entered\n";

    cout << "\nCOMMAND: q\n";
    cout << "IN shell MODE ONLY: quit the program\n";

    cout <<"\nENVIRONMENT VARIABLE: BDBH_DATABASE\n";
    cout << "If the switch --database is not specified, the path to the database file is read from this variable\n";
    
    cout <<"\nENVIRONMENT VARIABLE: BDBH_ROOTA\n";
    cout << "If the switch --root is not specified when the command add is used, this variable is used instead\n";
    
    cout <<"\nENVIRONMENT VARIABLE: BDBH_ROOTX\n";
    cout << "If the switch --root is not specified when the command extract/ls is used, this variable is used instead\n";
    
    cout << "\nEXIT CODES (status returned by the program, printed in interactive mode)\n";
    cout << BDBH_ERR_OK << " OK\n";
    cout << BDBH_ERR_US << " Usage: did you read the doc ?\n";
    cout << BDBH_ERR_DB << " Berkeley Db error\n";
    cout << BDBH_ERR_OW << " This should work with the switch --overwrite\n";
    cout << BDBH_ERR_DR << " This command is not allowed for a directory or a link\n";
    cout << BDBH_ERR_NF << " The specified key was not found in the database\n";
    cout << BDBH_ERR_RE << " This should work with the switch --recursive\n";
    cout << "\n";
    
    cout << "\nONLY THE COMMAND add MAY BE RUN SIMULTANEOUSLY BY SEVERAL PROCESSES ON THE SAME DATABASE";
    cout << "\nBUT WITHOUT THE --overwrite SWITCH\n";
    cout << "Allowed switches:\n";
    cout << "   -h [ --help ]            produce help message\n";
    cout << "   -d [ --database ] arg    path to the bdb database\n";
    cout << "   -c [ --compress ]        only with create: files are compressed/decompressed when written/read\n";
    cout << "   -t [ --root ] arg        path to the root of the hierarchy\n";
    cout << "   -C [ --directory ] arg   with extract: The hierarchy is retrieved to this directory\n";
    cout << "   -r [ --recursive ]       with extract/add/ls: directories are treated recursively\n";
    cout << "   -o [ --overwrite ]       with extract/add: data or files may be overwritten\n";
    cout << "   -v [ --verbose ]         a lot of messages are displayed\n";
    cout << "   -l [ --long_list ]       with ls: long listing\n";
    cout << "   -S [ --size_sort ]       with ls: list sorted in size (from small to big)\n";
    cout << "   -R [ --reverse_list ]    with ls: reverse sort\n";
    cout << "   -L [ --level ] arg (=-1) with extract/ls/chmod: the max depth to dig into in the hierarchy\n";
    cout << "   --mode arg (=0)          with chmod: the mode to specify, as 0400 or\n";
    cout << "                            f0400 (files only) or d0500 (directories only)\n";
    cout << "   --value arg              with put: pass the value to put instead of reading from stdin\n";
    cout << "   --stamp arg              with shell: pass a stamp, will be returned with the status\n";
#if NOCLUSTER
#else
    cout << "   --cluster                You can write simulatenously from several nodes of a cluster\n";
#endif
}


