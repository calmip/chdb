#include <stdexcept>
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
