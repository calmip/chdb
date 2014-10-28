

//#include <iostream>
//#include <iterator>

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
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
#include <libgen.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <dirent.h>

int operator<(const Finfo& a, const Finfo& b) { return a.st_size < b.st_size; }

/** 
 * @brief Fill if possible the set of files to use
 * 
 */
void UsingFs::initInputFiles() const {
	string in_file = prms.getInFile();
	
	if (in_file != "") {
		ifstream in(in_file.c_str());
		if (!in.good()) {
			string msg = "ERROR - File could not be opened: ";
			msg += in_file;
			throw(runtime_error(msg));
		}

		// parse file: this should be a tsv file
		// lines starting by # are a comment and are ignored
		// empty lines are ignored
		// Lines with 2 fields and more are considered: (fields 2-)
		// Lines with 1 field are considered
		string tmp;
		while (in) {
			getline(in,tmp);
			if (tmp.size()!=0 && tmp[0]!='#') {
				size_t p=tmp.find_first_of('\t');
				if (p!=string::npos && p<tmp.length()-1) {
					input_files.insert(tmp.substr(p+1));
				} else {
					input_files.insert(tmp);
				}
			}
		}
	}
}

/**
   \brief Read the file names from the input directory and push them to files
          If files is already filled, do nothing
*/
void UsingFs::v_readFiles() {
	if (files.size()==0) {
		string top = prms.getInDir();
		string ext = prms.getFileType();
	
		// Fill if possible the set of files to use - If empty, ALL files of correct type will be considered
		initInputFiles();

		// fill the files private member, OR the files_tmp global
		size_t head_strip=top.length();
		if (top[head_strip-1]!='/') {
			head_strip += 1;
		}
		readDir(top,head_strip);
	}
}

/** 
 * @brief Check the name extension versus the required extension (file type)
 * 
 * @param name 
 * 
 * @return true if the type is OK, false if not
 */
bool UsingFs::isCorrectType(const string & name) const {
	string ext     = '.' + prms.getFileType();
	size_t ext_len = ext.length();
	size_t nme_len = name.length();
	if ( ext_len < nme_len ) {
		string nme_ext = name.substr(nme_len-ext_len);
		return ( nme_ext == ext );
	} else {
		return false;
	}
}

/** 
 * 
 * @brief Prepare the f vector for a balanced distribution of jobs:
 *        1/ The f_info list is sorted from the longest to the shortest file
 *        2/ The file names are copied to f using an interleaved algorithm
 * 
 * @param f_info A list of Finfo objects (used to sort the files)
 * @param f      The final result
 *
 */

void UsingFs::buildBlocks(list<Finfo>& f_info,vector_of_strings&f) const {
	// sort files_tmp by sizes and copy the names to files
	f_info.sort();

	// Some parameters
	size_t nb_files   = f_info.size();
	size_t nb_slaves;
	if (comm_size>1) {
		nb_slaves = comm_size - 1;
	} else {
		throw(logic_error("ERROR - setRank has not been called"));
	}
	
	size_t block_size = prms.getBlockSize();
	size_t slice_size = nb_slaves*block_size;
	size_t dim_f = 0;
	if (nb_files%slice_size == 0) {
		dim_f = nb_files;
	} else {
		dim_f = slice_size * (1 + nb_files/slice_size);
	}

	// reserve enough size for f and init to ""
	f.clear();
	f.reserve(dim_f);
	f.assign(dim_f,(string)"");

	// Keep the file names to the vector files, reversing the order and interleaving them
	// Ex. 40 files, 4 slaves, block_size=5 see test usingFsSortFiles1
	// 1st slice = | 0  4  8 12 16 | 1  5  9 13 17|  2  6 10 14 18|  3  7 11 15 19|  4 blocks
	// 2nd slice = |20 24 28 32 36 |21 25 29 33 37| 22 26 30 34 38| 23 27 31 35 39|  4 blocks
	
	// Ex. 25 files, 4 slaves, block_size=5 see test usingFsSortFiles2
	// slice_size=20, 2 slices, dim_f=40
	// 1st slice = | 0  4  8 12 16 | 1  5  9 13 17|  2  6 10 14 18|  3  7 11 15 19|  4 blocks
	// 2nd slice = |20 24 "" "" "" |21 "" "" "" ""| 22 "" "" "" ""| 23 "" "" "" ""|  4 blocks (with holes)
	
	size_t i=0;
	for (list<Finfo>::reverse_iterator ri=f_info.rbegin();ri!=f_info.rend();++ri,++i) {
		size_t k = slice_size * (i / slice_size); // 0, 20
		k += block_size * (i % nb_slaves);        // k += 0, 5, 10, 15
		k += (i%slice_size) / nb_slaves;          // k += 0, 1, 2, 3, 4
		f[k] = ri->name;
	}
}

/**
   \brief Read the directory, and if a file with the correct type is found, push it in files (private member)
          If in files sort mode, files is sorted correctly
*/

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

/** 
 * @brief Execute a command through executeSystem and return the exit status of the command
 *        We pass the out_pathes vector to create the output directories if necessary
 * 
 * @param cmd 
 * @param out_pathes 
 * 
 * @return the command exit status
 *
 */	
//#include <iostream>
int UsingFs::executeExternalCommand(const string& cmd,const vector_of_strings& out_pathes) const {

	// Create the subdirectories if necessary
	for (size_t i=0; i<out_pathes.size(); ++i) {
		findOrCreateDir(out_pathes[i]);
	}
//	cerr << "COUCOU " << cmd << "\n";

	return callSystem(cmd);
}

/** 
 * @brief Make the output directory, store the name to output_dir, throw an exception if error
 *        If rank_flg is true, the rank is taken into account (probably called by a slave)
 *
 * @param rank_flg If true, NOTHING IS CREATED
 * @param rep_flg If true, remove the directory if it already exists
 * 
 */
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

/** 
 * @brief Make a temporary output directory only if tmp is specified
 *        Init temp_output_dir
 *        If no tmp, only init temp_output_dir to output directory and return ""
 * 
 * @return The tmpdir name
 */
string UsingFs::makeTempOutDir() {
	string output_dir = prms.getOutDir();
	string tmpdir="";
	if (prms.isTmpDir()) {
		string tmp = prms.getTmpDir();
		tmpdir = tmp + '/' + output_dir;
		tmpdir += "_XXXXXX";
		char* tmpdir_c = (char*)malloc(tmpdir.length()+1);
		strcpy(tmpdir_c,tmpdir.c_str());
		tmpdir = mkdtemp(tmpdir_c);
		free(tmpdir_c);
		temp_output_dir=tmpdir;
	} else {
		temp_output_dir=output_dir;
	}
	return tmpdir;
}

/** 
 * @brief Consolidate the output, ie copy to the output directory the hierarchy created in the temporary.
 *        
 * @note -if temporary and output directory are the same, nothing is done
 * @note -If already called, nothing is done 
 * 

 */


/** 
 * @brief Consolidate output data from a directory to the output directory
 *        Files are copied from the source directory and it is removed
 * 
 * @param path Path to the directory we want to consolidate
 *             If "", we use the temporary directory
 *
 */
#include <iostream>
void UsingFs::consolidateOutput(const string& out_dir) const {
	string temp_out = (out_dir.length()==0) ? getTempOutDir() : out_dir;
	string out      = getOutDir();

	// output directory same directory as temp_out nothing to do !
	if (temp_out!=out) {
		// If directory to consolidate exists
		struct stat sts;
		if (stat(temp_out.c_str(), &sts)==0) {
			string cmd = "/bin/cp -a ";
			cmd += temp_out;
			cmd += "/* ";
			cmd += out;
			
			// We do not want any exception to escape from this function !
			callSystem(cmd,false);
			
			// remove temporary directory, do not ignore the error
			cmd = "rm -r ";
			cmd += temp_out;
			callSystem(cmd,false);
		}
	}
}

/** 
 * @brief Find or create the directory part of the path name
 * 
 * @param p The path name
 *
 * @exception throw an exception if the directory cannot be created
 */

void UsingFs::findOrCreateDir(const string & p) const {
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

