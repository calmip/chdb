#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
using namespace std;

#include <libgen.h>
#include <cstring>
#include <cerrno>
#include "system.hpp"

/** 
 * @brief Execute a command through system, return the exit status or thow an exception
 *        If there is a fork error (sts=-1) or a not executable /bin/sh error (127), we retry NUMBER_OR_RETRIES times
 *        before giving up, BUT we write a message to cerr
 * 
 * @param cmd 
 * @param err_flg (default=false) If true, an exception is thrown if the command returns an error
 * 
 * @return The return status of the command, OR -1 (coud not fork) OR 127 (could not execute /bin/sh)
 *
 */

//#define DEBUG_SYSTEM 1

int callSystem(string cmd, bool err_flg) {
#ifdef DEBUG_SYSTEM
	cerr << "DEBUG - CALLING callSystem cmd=" << cmd << '\n';
#endif

	int retry = NUMBER_OF_RETRIES;
	bool should_retry;
	int sts  = 0;
	int csts = 0;
	do {
		sts = system(cmd.c_str());
		csts= WEXITSTATUS(sts);
		if (sts==-1 || csts==127) {
			string host;
			getHostName(host);
			string msg = "WARNING ON ";
			msg += host;
			msg += " - system(";
			msg += cmd;
			msg += ") - ";
			if (sts==-1) {
				msg += "returned -1";
				csts = sts;
			} else {
				msg += "could not execute cmd";
			}
			cerr << msg << '\n';
			should_retry = true;

			// Choose a sleep duration between 0 and 1 s
			unsigned int duration = 1000 * (1.0 * random())/RAND_MAX;
			sleepMs(duration);
		} else {
			should_retry = false;
		}
		retry--;
	} while (should_retry && retry>0);

	if (csts!=0 && err_flg) {
		ostringstream err;
		err << "ERROR ! " << cmd << " returned an error: " << csts;
		throw(runtime_error(err.str()));
	}
	return csts;
}

/** 
 * @brief Call gethostname and put the result in a string
 * 
 * @param[out] h 
 */
void getHostName(string& h) {
	char* host_name = (char*) malloc(50);
	gethostname(host_name,50);
	h = host_name;
	free(host_name);
}

/** 
 * @brief Call get_current_dir_name and put the result in a string
 * 
 * @param[out] d
 */
void getCurrentDirName(string& d) {
	const char* d_c = get_current_dir_name();
	d = (string) d_c;
	free((void*)d_c);
}


/** 
 * @brief Sleep duration, counted in milliseconds
 * 
 * @param duration 
*/
void sleepMs(unsigned int duration) {
	struct timespec req;
	req.tv_sec  = 0;
	req.tv_nsec = 1000000*duration; // 1 ms = 1000000 ns
	nanosleep(&req,NULL);
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
 * @brief Returns true if string ends with the extension
 * 
 * @param name 
 * @param ext 
 * 
 * @return 
 */
bool isEndingWith(const string& name, const string& ext) {
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
 * @brief Returns true if string begins with the heading
 * 
 * @param string
 * @param heading
 * 
 * @return 
 */
bool isBeginningWith(const string& name, const string& heading) {
	size_t heading_len = heading.length();
	size_t nme_len = name.length();
	if ( heading_len <= nme_len ) {
		string nme_head = name.substr(0,heading_len);
		return ( nme_head == heading );
	} else {
		return false;
	}
}

/**
 * @brief replace a template with value
 * 
 * @param[in]  tmpl  The template to look for in text
 * @param[in]  value The value to replace with 
 * @param[out] text
 */
void replaceTmpl(const string& tmpl, const string& value, string& text) {
	size_t pos = 0;
	do {
		pos = text.find(tmpl,pos);

		if (pos != string::npos) {
			text.replace(pos,tmpl.length(),value);
		}
	} while(pos != string::npos);
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
 * @brief Create a directory, throw a runtime_error if fail
 * 
 * @param dir
 * @param mode
 */
void mkdir (const string& dir, mode_t mode) {
	int sts = mkdir(dir.c_str(), mode);
	if (sts != 0) {
		string msg="ERROR - Cannot create directory ";
		msg += dir;
		msg += " - Error= ";
		msg += strerror(errno);
		throw(runtime_error(msg));
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

