#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
using namespace std;

#include <libgen.h>
#include <cstring>
#include "system.hpp"

/** 
 * @brief Execute a command through system, return the exit status or thow an exception
 * 
 * @param cmd 
 * @param err_flg (default=false) If true, an exception is thrown if the command returns an error
 * 
 * @return The return status of the command
 * @exception throw a logic_error exception if system itself produce an error, even when err_flg is false
 *
 */

//#define DEBUG_SYSTEM 1

int callSystem(string cmd, bool err_flg) {
#ifdef DEBUG_SYSTEM
	cerr << "DEBUG - CALLING callSystem cmd=" << cmd << '\n';
#else
	cmd += " 2>/dev/null";
#endif
	
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
		throw(logic_error(msg));
	}
	if (csts!=0 && err_flg) {
		ostringstream err;
		err << "ERROR ! " << cmd << " returned an error: " << csts;
		throw(runtime_error(err.str()));
	}
	return csts;
}

/** 
 * @brief parse a file path in useful components
 * 
 * @param path      (/path/to/A.TXT)
 * @param[out] dir  (/path/to)
 * @param[out] name (A.TXT) 
 * @param[out] base (A)
 * @param[out] ext  (TXT)
 */
void parseFilePath(const string& path, string& dir, string& name, string& base, string& ext) {

	char*  file_path = (char*) malloc(path.length()+1);
	strcpy(file_path,path.c_str());

	dir  = dirname(file_path);
	strcpy(file_path,path.c_str());
	name = basename(file_path);
	free(file_path);

	if (name.length()!=0) {
		if (name[0] != '.') {
			size_t dot = name.find_last_of('.');
			base = (dot!=string::npos)?name.substr(0,dot):name;
			ext  = (dot!=string::npos && dot!=name.length()-1)?name.substr(dot+1):"";
		} else {
			base = "";
			ext  = (name.length()!=1)?name.substr(1):"";
		}
	} else {
		base="";
		ext="";
	}
}

/** 
 * @brief Split a string using ',' as delimiter
 * 
 * @param s 
 * 
 * @return The splitted string
 */
vector_of_strings split(const string& s) {
	vector_of_strings rvl;
	size_t opos=0;
	size_t pos =s.find_first_of(',',opos);
	while(pos != string::npos) {
		rvl.push_back(s.substr(opos,pos-opos));
		opos=pos+1;
		// if , is the last character
		if (opos==s.length()) break;
		
		// search next ,
		pos = s.find_first_of(',',opos);
	};
	// if , is NOT the last character push the remaining string
	if (opos!=s.length()) {
		rvl.push_back(s.substr(opos));
	}
	return rvl;
}

/** 
 * @brief returns true if the file or directory exists
 * 
 * @param f 
 * 
 * @return 
 */
bool fileExists(const string &f) {
	struct stat status;
	if (stat(f.c_str(), &status)==0) {
		return true;
	} else {
		return false;
	}
}

/** 
 * @brief Write in a Buffer for sending a block of file names
 *        We store the number of strings (as an integer), then the strings
 *        Storing three files, little endian: 3000xxx\0xxx\0xxx\0
 * 
 * @param[in] files_names A vector of file names
 * @param[in] bfr The buffer (should be already allocated)
 * @param[in] bfr_size The buffer size
 * @param[out] data_size The length of data stored
 * @exception A runtime_exception is thrown if the buffer is too small
 * 
 */
void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size ) {
	data_size=sizeof(int);
	for (size_t i=0; i<file_pathes.size(); ++i) {
		data_size += file_pathes[i].length()+1;
	}

	if (data_size > bfr_size) {
		throw(runtime_error("ERROR - Buffer too small"));
	}

	size_t vct_sze = file_pathes.size();
	memcpy(bfr,(void*)&vct_sze,sizeof(int));
	size_t offset=sizeof(int);
	for (size_t i=0;i<vct_sze; ++i) {
		strcpy((char*)bfr+offset,file_pathes[i].c_str());
		offset += file_pathes[i].length()+1;
	}
}

/** 
 * @brief Create a vector of file names from a receive buffer
 * 
 * @param[in]  bfr         The buffer
 * @param[out] data_size   The size of data read in bfr
 * @param[out] files_names 
 */
void bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& file_pathes) {
	int sze=0;
	memcpy((void*)&sze,bfr,sizeof(int));
	size_t l_tmp = sizeof(int);
	file_pathes.clear();
	for (int i=0; i<sze; ++i) {
		char const* b_tmp = (char const *)bfr+l_tmp;
		file_pathes.push_back(b_tmp);
		l_tmp += strlen(b_tmp)+1;
	}
	data_size = l_tmp;
}

