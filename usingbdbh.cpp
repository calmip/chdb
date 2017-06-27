

//#include <iostream>
//#include <iterator>

// See L 47 - Bullshit here
#include <mpi.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>
#include <cstring>
#include <set>
#include <fstream>
#include <sstream>
using namespace std;

//#include "command.h"
#include "system.hpp"
#include "usingbdbh.hpp"
#include "bdbh/command.hpp"
#include "bdbh/ls.hpp"
#include "bdbh/read.hpp"
#include "bdbh/create.hpp"
#include "bdbh/write.hpp"
#include "bdbh/merge.hpp"

//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
#include <libgen.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <dirent.h>

typedef auto_ptr<bdbh::Command> Command_aptr;

UsingBdbh::UsingBdbh(const Parameters& p):Directories(p),input_bdb(NULL),output_bdb(NULL),temp_bdb(NULL),need_consolidation(false) {
	bdbh::Initialize();
	// Switch in-memory: used ONLY by rank 0, if using bdbh
	bool in_memory = false;
	
	// @todo - We cannot use the rank defined by the scheduler because the Scheduler must be created AFTER th e Directory ! Bull shit here !
	int mpi_rank;
	MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
	if (mpi_rank == 0 && prms.isInMemory()) in_memory = true;
	input_bdb = (BerkeleyDb_aptr) new bdbh::BerkeleyDb(prms.getInDir().c_str(),BDBH_OREAD,false,false,in_memory);
}

// consolidateOutput may throw an exception (if incompletly initalized) - Ignore it
UsingBdbh::~UsingBdbh() {
	try {
		consolidateOutput(true);
	} catch (exception& e){
		cerr << "Process rank " << rank << " - ";
		cerr << "EXCEPTION CATCHED DURING UsingBdbh DESTRUCTOR: \n" << e.what() << '\n';
	};
	bdbh::Terminate();
}

class PushFiles: public bdbh::LsObserver {
public:
	PushFiles(const string& r, const string& t, vector_of_strings& vs, const set<string>&ss):root(r),file_type(t),files(vs),input_files(ss),in_files_empty(ss.empty()){};
	void Update(string name,const bdbh::Mdata& mdata) {
		// Strip the root
		// if nothing stripped or bad type give up
		// if input_files not empty check the file is to be kept

		//cerr << "coucou (root,name) " << root << ',' << name << "\n";
		if (bdbh::StripLeadingStringSlash(root,name) || name.length()==0) {
			if (isEndingWith(name,file_type)) {
				if (in_files_empty || input_files.find(name)!=input_files.end()) {
					files.push_back(name);
				}
			}
		}
	};

private:
	string root;
	string file_type;
	vector_of_strings& files;
	const set<string>& input_files;
	const bool in_files_empty;
};

class PushFilesMeta: public bdbh::LsObserver {
public:
	PushFilesMeta(const string& r, const string& t, list<Finfo>& ls, set<string>ss):root(r),file_type(t),files(ls),input_files(ss),in_files_empty(ss.empty()){};
	void Update(string name,const bdbh::Mdata& mdata) {

		// Strip the root
		// if nothing stripped or bad type give up
		// if input_files not empty check the file is to be kept
		if (bdbh::StripLeadingStringSlash(root,name) || name.length()==0) {
			if (isEndingWith(name,file_type)) {
				if (in_files_empty || input_files.find(name)!=input_files.end()) {
					files.push_back(Finfo(name,mdata.size));
				}
			}
		}
	};

private:
	string root;
	string file_type;
	list<Finfo>& files;
	set<string>& input_files;
	const bool in_files_empty;
};

/**
   \brief Read the file names from the input directory and push them to files
          If files is already filled, do nothing
*/
void UsingBdbh::v_readFiles() {
	if (files.size()==0) {
		string top = prms.getInDir();
		//string root=top.substr(0,top.length()-3); // input.db -> input
		string root = "";
		string ext = ".";
		ext += prms.getFileType();
	
		// Fill if possible the set of files to use - If empty, ALL files of correct type will be considered
		initInputFiles();

		//cerr << "INPUT_FILES " << input_files.size() << "\n";
		//for (set<string>::iterator i=input_files.begin();i!=input_files.end();++i) { cerr << "F-> " << *i << " "; };
		//cerr << "FIN\n";
		// fill the files private member, OR the files_tmp global
		//size_t head_strip=top.length();
		//if (top[head_strip-1]!='/') {
		//	head_strip += 1;
		//}
   

		//const char* args[] = {"--database",top.c_str(),"ls"};
		//bdbh::Parameters bdbh_prms(3,args);
		bdbh::Parameters bdbh_prms;
		bdbh::Ls ls_cmd(bdbh_prms,*input_bdb.get());

		// Attach the observer to an observer object, then execute the ls
		if ( prms.isSizeSort() ) {
			list<Finfo> files_tmp;
			PushFilesMeta pf (root,ext,files_tmp,input_files);
			ls_cmd.AttachObserver(pf);
			ls_cmd.Exec();
			files.clear();
			buildBlocks(files_tmp,files);
		} else {
			PushFiles pf(root,ext,files,input_files);
			ls_cmd.AttachObserver(pf);
//			cerr << "INPUT_FILES " << input_files.size() << "\n";
//			for (set<string>::iterator i=input_files.begin();i!=input_files.end();++i) { cerr << "F-> " << *i << " "; };
//			cerr << "FIN\n";
			
			ls_cmd.Exec();
			sort(files.begin(),files.end());
		}
	}
}


/**
   \brief Read the directory, and if a file with the correct type is found, push it in files (private member)
          If in files sort mode, files is sorted correctly
*/
/*
void UsingFs::readDir(const string &top,size_t head_strip) const {
	list<Finfo> files_tmp;
	readDirRecursive(top,head_strip,files_tmp,prms.isSizeSort());
	if (prms.isSizeSort()) {
		files.clear();
		buildBlocks(files_tmp,files);
	} else {
		sort(files.begin(),files.end());
	}
}
*/
/** 
 * @brief Read the directory, and if a file with the correct type is found, push it in
 *             - files        (private member)
 *			   - OR files_tmp (passed by parameter)
 *		  If the name is longer than FILEPATH_MAXLENGTH throw an exception
 *        If a subdirectory is found, this function is recursively called again
 *        If the entry is anything else (symlink, etc) skip it
 *        The parameter head_strip may be used to strip input directory name from the file names
 * 
 * @param top 
 * @param head_strip 
 * @param[out] files_tmp
 */
/*
void UsingFs::readDirRecursive(const string &top,size_t head_strip,list<Finfo>& files_tmp,bool is_size_sort) const {
	bool in_files_empty=input_files.empty();
	DIR* fd_top=opendir(top.c_str());
	struct dirent* dir_entry=NULL;
	do {
		dir_entry = readdir(fd_top);
		if (dir_entry != NULL)
		{
			// Skip . and ..
			if (dir_entry->d_name[0]=='.' && dir_entry->d_name[1]=='\0') 
				continue;
			if (dir_entry->d_name[0]=='.' && dir_entry->d_name[1]=='.' && dir_entry->d_name[2]=='\0') 
				continue;
			
			string file_name   = top + '/' + dir_entry->d_name;
			string s_file_name = file_name.substr(head_strip); 
			if (s_file_name.length() > FILEPATH_MAXLENGTH) {
				string msg = "ERROR - Filename too long: ";
				msg += s_file_name + '\t';
				msg += "Please increase FILEPATH_MAXLENGTH and recompile";
				throw (runtime_error(msg));
			};
				
			struct stat st_bfr;
			int rvl = lstat(file_name.c_str(),&st_bfr);
			if (rvl==-1) {
				string msg = "ERROR - Cannot read the file ";
				msg += file_name;
				throw(runtime_error(msg));
			}

            // If in size sort, we use files_tmp for temporary storage
			if (S_ISREG(st_bfr.st_mode)) {
				if (isCorrectType(s_file_name))
					if (in_files_empty || input_files.find(s_file_name)!=input_files.end()) {
						if (is_size_sort) {
							Finfo tmp_f(s_file_name,st_bfr.st_size);
							files_tmp.push_back(tmp_f);
						} else {
							files.push_back(s_file_name);
						}
					}
			} else if (S_ISDIR(st_bfr.st_mode)) {
				readDirRecursive(file_name,head_strip,files_tmp,is_size_sort);
			}
		}
	} while ( dir_entry != NULL );
	closedir(fd_top);
}
*/
/** 
 * @brief Execute a command through executeSystem and return the exit status of the command
 *        We pass the out_pathes vector to create the output directories if necessary
 * 
 * @param in_pathes
 * @param cmd 
 * @param out_pathes 
 * 
 * @return the command exit status
 *
 */	
//#include <iostream>
/** 
 * @brief Execute the external command, using complete in_pathes as input and complete out_pathes as output
 *        The in_pathes are read from the database and written to the temp directory
 *        (NOTE - Only in_pathes[0] is considered yet)
 *        The command is executed
 *        If the return value is 0, the out_pathes are written to the output database and removed from the temp dir
 * 
 * @pre The cmd is ready to be executed (templates subsititution already done)
 * @param in_pathes  input pathes, relative to the input directory (ie inputdir/A/B.txt ==> A/B.txt
 * @param cmd 
 * @param out_pathes output pathes, relative to the output directory (ie inputdir.out/A/B.out ==> A/B.out)
 * 
 * @exception Throw a logic_error exception if anything is wrong with bdbh (read or write)
 * @return The return value of callSystem
 */
int UsingBdbh::executeExternalCommand(const vector_of_strings& in_pathes,const string& cmd,const vector_of_strings& out_pathes, const string&, const string&) {

	// something new in output directory --> we need consolidation !
	need_consolidation = true;

	// Create the input pathes in tmpdir
	string in_top  = prms.getInDir();
	string in_root = in_top.substr(0,in_top.length()-3); // removing .db from end
	
	string out_top = prms.getOutDir();
	string out_root= out_top.substr(0,out_top.length()-3); // removing .db from end

	// Read the input pathes from the database, and copy them to the temporary input directory
	string temp_input_dir = getTempInDir();
//	const char* args[] = {"--database",in_top.c_str(),"--root",in_root.c_str(),"--directory",temp_input_dir.c_str(),"extract","--recursive",in_pathes[0].c_str()};
//	const char* args[] = {"--root",in_root.c_str(),"--directory",temp_input_dir.c_str(),"--recursive",in_pathes[0].c_str()};
	const char* args[] = {"--root","","--directory",temp_input_dir.c_str(),"--recursive",in_pathes[0].c_str()};
	bdbh::Parameters bdbh_prms_r(6,args);
	bdbh::Read read_cmd(bdbh_prms_r,*input_bdb.get());
	read_cmd.Exec();
	int bdbh_rvl_r = read_cmd.GetExitStatus();
	if (bdbh_rvl_r != 0) {
		ostringstream out;
		out << "ERROR - could not extract file " << in_pathes[0] << " from the database. Status=" << bdbh_rvl_r;
		throw(logic_error(out.str()));
	}

	// Create the subdirectories if necessary
	// If out_pathes() starts with out_dir, it's OK. If not, complete them to create the subdirectories
	// Create also a version WITHOUT out_dir, it will be useful to store to the database

	string temp_out_dir            = getTempOutDir();
	vector_of_strings l_out_pathes = out_pathes;     // Local copy, because the parameter is const
	for (size_t i=0; i<l_out_pathes.size(); ++i) {
		string f;
		if ( isBeginningWith(l_out_pathes[i],temp_out_dir) ) {
			f = l_out_pathes[i];
			bdbh::StripLeadingStringSlash(temp_out_dir,l_out_pathes[i]);
		} else {
			f = temp_out_dir;
			f += '/';
			f += l_out_pathes[i];
		}
		findOrCreateDir(f);
	}

	int rvl = callSystem(cmd);

	// if rvl == 0, we save to the database the output files before returning
	//if (rvl==0) {
		vector_of_strings arg;            // The arg to write output files to the database

		//arg.push_back("--database");
		//arg.push_back(temp_db_dir);
		arg.push_back("--root");
		arg.push_back(out_root);
		arg.push_back("--directory");
		arg.push_back(temp_out_dir);
		arg.push_back("--recursive");
		arg.push_back("--overwrite");
		//arg.push_back("--verbose");
		//arg.push_back("add");
		// If no output file created, we'll get an exception !

		bool path_exists=false;	          // False if nothing created (which is probably an error)

		// Keep track of the files to be stored and destroyed
		for (size_t i=0; i<l_out_pathes.size(); ++i) {
			if (fileExists(out_pathes[i])) {
				arg.push_back(l_out_pathes[i]);
				path_exists=true;
			}
		}

		if (path_exists==true) {

			// Store the outputfiles to the database
			bdbh::Parameters bdbh_prms_w(arg);
			bdbh::Write write_cmd(bdbh_prms_w,*temp_bdb.get());
			//cout << "INFO - WRITING DATA TO " << temp_db_dir << '\n';
			// TODO - Exception of GetExitStatus() ? Not clair
			write_cmd.Exec();
			int bdbh_rvl_w = write_cmd.GetExitStatus();
			if (bdbh_rvl_w != 0) {
				ostringstream out;
				out << "ERROR - could not save one of the output files to the database. Status=" << bdbh_rvl_w;
				out << "Files to be saved = ";
				for (size_t i=0; i<l_out_pathes.size(); ++i) {
					out << out_pathes[i] << ' ';
				}
				throw(logic_error(out.str()));
			}
			
			// Destroy the output files/directories
			for (size_t i=0; i<out_pathes.size(); ++i) {
				string cmd = "rm -rf ";
				cmd += out_pathes[i];
				callSystem(cmd,false);

				//rvl = unlink( out_pathes[i].c_str() );
				//if (rvl != 0) {
				//	cerr << "WARNING - Could not remove file " << out_pathes[i] << " - " << strerror(rvl) << "\n";
				//}
			}
			
			// Destroy the input file(s)
			// Destroy only in_pathes[0]
			string f = temp_input_dir + '/' + in_pathes[0];
			unlink ( f.c_str());
		}
		return rvl;
//	} else {
		//		return rvl;
//	}
}

/** 
 * @brief Make the output directory, store the name to output_dir, throw an exception if error
 *        If rank_flg is true, the rank is taken into account (probably called by a slave)
 *
 * @param rank_flg If true, NOTHING IS CREATED
 * @param rep_flg If true, remove the directory if it already exists
 * 
 */
/*
void UsingFs::makeOutDir(bool rank_flg, bool rep_flg) {
	output_dir = prms.getOutDir();
	if (rank_flg) {
		return;
	}

	// remove directory if rep_flg and directory already exists
	struct stat status;
	if (rep_flg && stat(output_dir.c_str(), &status)==0) {
		string cmd = "rm -r ";
		cmd += output_dir;
		callSystem(cmd);
	}
	
	int sts = mkdir(output_dir.c_str(), 0777);
	if (sts != 0) {
		string msg="ERROR - Cannot create directory ";
		msg += output_dir;
		msg += " - Error= ";
		msg += strerror(errno);
		throw(runtime_error(msg));
	}
}
*/
/** 
 * @brief Make the output directory, store the name to output_dir, throw an exception if error
 *        NOTE - OUTDIRPERSLAVE is IGNORED

 * @param rank_flg If true: append the rank to the directory name, 
 * @param rep_flg If true, remove the directory if it already exists
 * 
 */
void UsingBdbh::makeOutDir(bool rank_flg, bool rep_flg) {
	output_dir = prms.getOutDir();

	if (rank_flg) {
		ostringstream tmp;
		tmp << rank;
		output_dir += '.';
		output_dir += tmp.str();
	}
	
	// remove directory if rep_flg and directory already exists
	struct stat status;
	if (rep_flg && stat(output_dir.c_str(), &status)==0) {
		string cmd = "rm -r ";
		cmd += output_dir;
		callSystem(cmd);
	}

	// Only the MASTER creates the output directory, the slaves will create it only during consolidation
	if (rank == 0) {
		// throw a runtime error if directory already exists, else makes directory
		mkdir (output_dir);
	
		// Create and open an output BerkeleyDb
		output_bdb = (BerkeleyDb_aptr) new bdbh::BerkeleyDb(output_dir.c_str(),BDBH_OCREATE);
	
		//const char* args[] = {"--database",output_dir.c_str(),"create"};
		//bdbh::Parameters bdbh_prms(3,args);
		bdbh::Parameters bdbh_prms;
		bdbh::Create create_cmd(bdbh_prms,*output_bdb.get());
		create_cmd.Exec();
		int rvl = create_cmd.GetExitStatus();
		if (rvl != 0) {
			ostringstream out;
			out << "ERROR - could not create output " << output_dir << " database. Status=" << rvl;
			throw(logic_error(out.str()));
		}
	
		// Reopen the output db in write mode
		output_bdb.reset();
		output_bdb = (BerkeleyDb_aptr) new bdbh::BerkeleyDb(output_dir.c_str(),BDBH_OWRITE);
	}
}

/** 
 * @brief Consolidate the output, ie copy to the output directory the hierarchy created in the temporary.
 *        
 * @note -if temporary and output directory are the same, nothing is done
 * @note -If already called, nothing is done 
 * 

 */

/** 
 * @brief Make a temporary output directory 
 *        NOTE - We make THREE subdirectories: one for input, another for output, the last for db
 * 
 * @exception If tmp does not exist throw a bdbh exception
 */
void UsingBdbh::makeTempOutDir() {

	// Generating an exception if outdir is not yet initialized !
	string outdir = getOutDir();
	string tmpdir = "";
	string tmp    = (prms.isTmpDir())?prms.getTmpDir():DEFAULT_BDBH_TMP_DIRECTORY;
	if (fileExists(tmp)) {
		tmpdir = tmp + '/' + output_dir;
		tmpdir += "_XXXXXX";
		char* tmpdir_c = (char*)malloc(tmpdir.length()+1);
		strcpy(tmpdir_c,tmpdir.c_str());
		tmpdir = mkdtemp(tmpdir_c);
		free(tmpdir_c);
	} else {
		string msg="ERROR - directory ";
		msg += tmp;
		msg += " does not exist";
		throw(runtime_error(msg));
	}

	// Make the two subdirectories: "input" and "output"
	temp_dir        = tmpdir;
	temp_input_dir  = tmpdir + "/input";
	temp_output_dir = tmpdir + "/output";
	temp_db_dir     = tmpdir + "/db";
	mkdir(temp_input_dir);
	mkdir(temp_output_dir);
	mkdir (temp_db_dir);

	// Create and open the database
	temp_bdb = (BerkeleyDb_aptr) new bdbh::BerkeleyDb(temp_db_dir.c_str(),BDBH_OCREATE);
	
	//const char* args[] = {"--database",temp_db_dir.c_str(),"create"};
	//bdbh::Parameters bdbh_prms(3,args);
	bdbh::Parameters bdbh_prms;
	bdbh::Create create_cmd(bdbh_prms,*temp_bdb.get());
	create_cmd.Exec();
	int rvl = create_cmd.GetExitStatus();
	if (rvl != 0) {
		ostringstream out;
		out << "ERROR - could not create temp " << output_dir << " database. Status=" << rvl;
		throw(logic_error(out.str()));
	}

	// Reopen the output db in write mode
	temp_bdb.reset();
	temp_bdb = (BerkeleyDb_aptr) new bdbh::BerkeleyDb(temp_db_dir.c_str(),BDBH_OWRITE);
}

/** 
 * @brief Consolidate output data from a directory to the output directory
 *        Files are copied from the source directory then it is removed
 * 
 * @param from_tmp If true, consolidate from temporary directory, else consolidate from path
 * @param path Used only if from_tmp==false: path to the directory we want to consolidate
 *             If from_tmp==false and path=="", return without doing anything
 *
 */
void UsingBdbh::consolidateOutput(bool from_tmp, const string& path) {
	// if from_tmp: exit if nothing to consolidate
	// else: consolidate anyway, as we cannot know if it is useful or not
	if (from_tmp==true && need_consolidation==false) {
		return;
	}
	
	// mark the dir to consolidated status
	need_consolidation = false;

	string src_dir, src_dir_db;
	
	// tmp directory has the following structure:
	// some/path/       ==> src_dir
	// some/path/db     ==> src_dir_db
	// some/path/input
	// some/path/output
	if (from_tmp) {
		src_dir = getTempOutDir();
		//src_dir_db = src_dir + "/db";
		src_dir_db = getTempDbDir();
		temp_bdb.get()->Sync();
		temp_bdb.reset();
	} else {
		src_dir    = path;
		src_dir_db = path;		
	}

	if (src_dir.size()==0) {
		return;
	}

	string dst_dir = getOutDir();

	// If dst_dir directory is same directory as src_dir nothing to do !
	if (src_dir!=dst_dir) {

		struct stat sts;
		if (stat(src_dir.c_str(), &sts)==0) {
			if (prms.isVerbose()) {
				cerr << "INFO - rank " << rank << " is now consolidating data " << src_dir_db << " to " << dst_dir << "\n";
			}
			
			// If destination directory exists, merge source directory to it
			if (stat(dst_dir.c_str(), &sts)==0) {
					
	
				//const char* args[] = {"--database",getOutDir().c_str(),"merge",src_dir_db.c_str()};
				const char* args[] = {src_dir_db.c_str()};
				bdbh::Parameters bdbh_prms(1,args);
				bdbh::Merge merge_cmd(bdbh_prms,*output_bdb.get());
				merge_cmd.Exec();
				int rvl = merge_cmd.GetExitStatus();
				if (rvl != 0) {
					ostringstream out;
					out << "ERROR - could not merge database " << src_dir << " to " << dst_dir << " Status=" << rvl;
					throw(logic_error(out.str()));
				}
	
				// Sync output data
				output_bdb.get()->Sync();
			}
		
			// If destination directory does not exist, we just have to cp
			else {
				string cmd = "cp -a " + src_dir_db + " " + dst_dir;
				callSystem(cmd,false);
			}
		} else {
			string msg = "ERROR - could not merge database ";
			msg += src_dir;
			msg += " to ";
			msg += dst_dir;
			msg += dst_dir;
			msg += " (";
			msg += src_dir;
			msg += " does not exist)";
			throw(logic_error(msg));
		}

		// remove temporary directory, ignore the error
		// It temporary, we remove db, input, output
		string to_remove;
		if (from_tmp) {
			to_remove = src_dir;
			
		// Else, we remove only db directory
		} else {
			to_remove = src_dir_db;
		}
		
		/*
		if ( prms.isVerbose() ) {
			cerr << "INFO - rank " << rank << " is now removing " << to_remove << "\n";
		}
		string cmd = "rm -rf ";
		cmd += to_remove;
		callSystem(cmd,false);
		*/ 
	}
}

/** 
 * @brief Find or create the directory part of the path name
 * 
 * @param p The path name
 *
 * @exception throw an exception if the directory cannot be created
 */

void UsingBdbh::findOrCreateDir(const string & p) {
	string d,n,b,e;
	parseFilePath(p,d,n,b,e);

	// avoid stressing the filesystem if possible !
	if (found_directories.find(d)!=found_directories.end()) {
		return;
	}

	// Was not already found, search it on the fs
	struct stat st;
	int sts = stat(d.c_str(),&st);

	// directory found: remember for next time, and return
	if (sts==0 && S_ISDIR(st.st_mode)) {
		found_directories.insert(d);
		return;
	}

	bool exc_flg = false;

	// something found but not a directory: exception !
	if (sts==0 && !S_ISDIR(st.st_mode)) {
		exc_flg = true;	}

	// something in the path is NOT a directory: exception !
	if (sts!=0 && errno==ENOTDIR) {
		exc_flg = true;
	}

	if (exc_flg) {
		string msg="ERROR WITH DIRECTORY CREATION: ";
		msg += d;
		throw(runtime_error(msg));
	}

	// some component of the path does not exist: recursive call, then create directory
	if (sts!=0) {
		if (errno==ENOENT) {
			findOrCreateDir(d);
			sts = mkdir(d.c_str(),0777);

			// directory successfully created
			if (sts == 0) {
				found_directories.insert(d);
				return;
			}

			// directory already exists: if was already created by some other process
			if (sts!=0 && errno==EEXIST) {
				found_directories.insert(d);
				return;
			}

			// Any other issue: throw an exception !
			string msg="ERROR WITH DIRECTORY CREATION: ";
			msg += d;
			msg += " - ";
			msg += strerror(errno);
			throw(runtime_error(msg));

			// If the directory could not be created, we just ignore the result of mkdir
			// If probably means there is a race condition with other processes
			// We consider it was created anyway, but we do not store the directory name
		}
	}
}

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/

