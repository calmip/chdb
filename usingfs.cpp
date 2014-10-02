

//#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>

//#include "command.h"
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
	Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};
int operator<(const Finfo& a, const Finfo& b) { return a.st_size < b.st_size; }

/* This is a temporary global variable used when we have to sort the files */
list<Finfo> files_tmp;

/**
   \brief Retrieve the file names from the input directory and push them to files
          If files is already filled, we just return the vector
*/

const vector_of_strings& UsingFs::getFiles() const {
	string top = prms.getInDir();
	string ext = prms.getFileType();
	
	if (files.size()==0) {
		// fill the files private member, OR the files_tmp global
		readDir(top);

		// if sorted by size, sort the temporary and copy the names to files
		files_tmp.sort();
		for (list<Finfo>::iterator i=files_tmp.begin();i!=files_tmp.end();++i) {
			files.push_back(i->name);
		}

		// do not waste memory
		files_tmp.clear();
	}
	return files;
}

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

void UsingFs::readDir(const string &top) const {
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
			
			string file_name = top + '/' + dir_entry->d_name;
			if (file_name.length() > FILEPATH_MAXLENGTH) {
				string msg = "ERROR - Filename too long: ";
				msg += file_name + '\t';
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
				if (isCorrectType(file_name)) {
					if (prms.isSizeSort()) {
						Finfo tmp_f(file_name,st_bfr.st_size);
						files_tmp.push_back(tmp_f);
					} else {
						files.push_back(file_name);
					}
				}
			} else if (S_ISDIR(st_bfr.st_mode)) {
				readDir(file_name);
			}
		}
	} while ( dir_entry != NULL );
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

