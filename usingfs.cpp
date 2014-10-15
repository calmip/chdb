

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
using namespace std;

//#include "command.h"
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
#include <libgen.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <dirent.h>

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
	Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};
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
   \brief Read the directory, and if a file with the correct type is found, push it in:
               - files (private member)
			   - OR files_tmp (global temporary)
		  If the name is longer than FILEPATH_MAXLENGTH throw an exception

          If a subdirectory is found, this function is recursively called again
		  If the entry is anything else (symlink, etc) skip it
*/

void UsingFs::readDir(const string &top,size_t head_strip) const {
	list<Finfo> files_tmp;
	readDirRecursive(top,head_strip,files_tmp,prms.isSizeSort());
	if (prms.isSizeSort()) {
		// if sorted by size, sort files_tmp and copy the names to files
		files_tmp.sort();
		for (list<Finfo>::iterator i=files_tmp.begin();i!=files_tmp.end();++i) {
			files.push_back(i->name);
		}
	}
}

/** 
 * @brief Read the directory, and if a file with the correct type is found, push it in:
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
 * @brief Execute a command through system and return the exit status of the command
 *        We pass the out_pathes vector to create the output directories if necessary
 * 
 * @param cmd 
 * @param out_pathes 
 * 
 * @return the command exit status
 *
 * @exception throw a runtime_error exception if system itself produce an error

 */	
int UsingFs::executeExternalCommand(const string& cmd,const vector_of_strings& out_pathes) const {

	// Create the subdirectories if necessary
	for (size_t i=0; i<out_pathes.size(); ++i) {
		findOrCreateDir(out_pathes[i]);
	}
	int sts = system(cmd.c_str());
	int csts= WEXITSTATUS(sts);
	if (sts==-1 || csts==127) {
		string msg = "ERROR - system(";
		msg += cmd;
		msg += ") - ";
		if (sts==-1) {
			msg += "returned -1";
		} else {
			msg += "could not execute cmd";
		}
		throw(runtime_error(msg));
	}
	return csts;
}
	
void UsingFs::makeOutputDir() const {
	string output_dir = prms.getOutDir();
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
 * @brief Find or create the directory part of the path name
 * 
 * @param p The path name
 *
 * @exception throw an exception if the directory cannot be created
 */

void UsingFs::findOrCreateDir(const string & p) const {
	char* file_path = (char*) malloc(p.length()+1);
	strcpy(file_path,p.c_str());
	string d = dirname(file_path);
	free(file_path);

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

