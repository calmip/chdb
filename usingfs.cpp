
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



/***
 *   \brief Check the parameters (member prms)
 * 
 *   Throw a runtime error if something wrong
 * 
 *******************/
void UsingFs::checkParameters() {}

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

		size_t head_strip=top.length();
		if (top[head_strip-1]!='/') {
			head_strip += 1;
		}
		readDir(top,head_strip);
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

			// Select the file only if in input_files... or if input_files is empty, select it anyway
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
 * @brief Execute the external command, using complete in_pathes as input and complete out_pathes as output
 * 
 * @pre The cmd is ready to be executed (templates subsititution already done)
 *
 * @param in_pathes input pathes, relative to the input directory (ie inputdir/A/B.txt ==> A/B.txt)
 * @param cmd 
 * @param out_pathes output pathes, relative to the output directory (ie outputdir/A/B.txt ==> A/B.txt)
 * @param work_dir    The work directory to change into
 * @param snippet     The script snippet to execute to build the environment
 * 
 * @exception Throw a logic_error if files cannot be read/written (should NOT happen)
 * @return The return value of callSystem
 */
int UsingFs::executeExternalCommand(const vector_of_strings& in_pathes,
									const string& cmd,
									const vector_of_strings& out_pathes,
									const string& work_dir,
									const string& snippet) {

    // throw a logic error if one of the input pathes is not readable
	string in_dir = getTempInDir();
	for (size_t i=0; i<in_pathes.size(); ++i) {
		string f = in_dir;
		f += '/';
		f += in_pathes[i];
		if (!fileExists(f)) {
			string msg = "ERROR - File does not exist:  ";
			msg += in_pathes[i];
			throw(logic_error(msg));
		}
	}

	// Create the subdirectories if necessary
	// If out_pathes() starts with out_dir, it's OK. If not, complete them !
	string out_dir = getTempOutDir();
	for (size_t i=0; i<out_pathes.size(); ++i) {
		string f;
		if ( isBeginningWith(out_pathes[i],out_dir) ) {
			f = out_pathes[i];
		} else {
			f = out_dir;
			f += '/';
			f += out_pathes[i];
		}
		findOrCreateDir(f);
	}

	// Create and change to work directory if requested
	string saved_dir = "";
	if (work_dir.length() != 0) {
		getCurrentDirName(saved_dir);
		string ok = work_dir + "/ok";  // dummy file because findOrCreateDir checks only the dir part
		findOrCreateDir(ok);
		int sts = chdir(work_dir.c_str());
		if (sts != 0 ) {
			string msg="ERROR - Cannot change to work directory ";
			msg += work_dir;
			msg += " - Error= ";
			msg += strerror(errno);
			throw(runtime_error(msg));
		}
	}

	// will be returned at the end
	int sts=0;

	// Execute the snippet, if any
	if (snippet.length() != 0) {
		string sni_cmd = "/bin/bash -c '";
		sni_cmd += snippet;
		sni_cmd += "'";
		sts = callSystem(sni_cmd);
	}

	// If snippet was successfull (or no snippet at all)
	if (sts==0) {
		// Change command for mpi instructions if necessary
		string command = cmd;
		buildMpiCommand(command);

		// Call command and keep value
		sts = callSystem(command);

		// Change to previous current directory if necessary
		// NB - Should work, we don't even check
		if (saved_dir.length() != 0) {
			chdir(saved_dir.c_str());
		}
	}

	// Return call status
	return sts;
}

/** 
 * @brief Make the output directory, store the name to output_dir, throw an exception if error
 *        The define OUTDIRPERSLAVE changes the behaviour of this function

 * @param rank_flg If true: if OUTDIRPERSLAVE  append the rank to the directory name, 
 *                          if OUTDIRPERSLAVE not defined create nothing
 * @param rep_flg If true, remove the directory if it already exists
 * 
 */
void UsingFs::makeOutDir(bool rank_flg, bool rep_flg) {
	output_dir = prms.getOutDir();

#ifdef OUTDIRPERSLAVE
	if (rank_flg) {
		ostringstream tmp;
		tmp << rank;
		output_dir += '.';
		output_dir += tmp.str();
	}
#else
	if (rank_flg) {
		return;
	}
#endif
	
	// remove directory if rep_flg and directory already exists
	struct stat status;
	if (rep_flg && stat(output_dir.c_str(), &status)==0) {
		string cmd = "rm -r ";
		cmd += output_dir;
		callSystem(cmd);
	}
	
	mkdir(output_dir);
}

/** 
 * @brief Make a temporary output directory only if tmp is specified
 *        Init temp_output_dir
 *        If no tmp, only init temp_output_dir to output directory and return ""
 * 
 *
 */
void UsingFs::makeTempOutDir() {

	// Generating an exception if outdir is not yet initialized !
	string outdir = getOutDir();
	string tmpdir="";
	if (prms.isTmpDir()) {
		string tmp = prms.getTmpDir();
		if (fileExists(tmp)) {
			tmpdir = tmp + '/' + output_dir;
			tmpdir += "_XXXXXX";
			char* tmpdir_c = (char*)malloc(tmpdir.length()+1);
			strcpy(tmpdir_c,tmpdir.c_str());
			tmpdir = mkdtemp(tmpdir_c);
			free(tmpdir_c);
			temp_output_dir=tmpdir;
		} else {
			temp_output_dir=outdir;
		}
	} else {
		temp_output_dir=outdir;
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
 * @brief Consolidate output data from a directory to the output directory
 *        Files are copied from the source (temporary) directory and it is removed
 *        If src and dst directoreis are same, nothing done
 * 
 * @param from_tmp If true, consolidate from temporary directory, else consolidate from path
 * @param path Used only if from_tmp==false: path to the directory we want to consolidate
 *             If from_tmp==false and path=="", return without doing anything
 *
 */
void UsingFs::consolidateOutput(bool from_tmp, const string& path) {
	string src_dir = (from_tmp) ? getTempOutDir() : path;
	if (src_dir.size()==0) {
		return;
	}
	string dst_dir      = getOutDir();

	// output directory same directory as src_dir nothing to do !
	if (src_dir!=dst_dir) {
		// If directory to consolidate exists
		// @todo - Optimization => mv if same volume, cp if different volumes !
		
		struct stat sts;
		if (stat(src_dir.c_str(), &sts)==0) {
			string cmd = "/bin/cp -a ";
			cmd += src_dir;
			cmd += "/* ";
			cmd += dst_dir;
			cmd += " 2> /dev/null";
			
			// We do not want any exception to escape from this function as it is called from destructor
			callSystem(cmd,false);
			
			// remove temporary directory, ignore the error
			cmd = "rm -r ";
			cmd += src_dir;
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

void UsingFs::findOrCreateDir(const string & p) {
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

