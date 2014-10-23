#include <stdexcept>
#include <cstdlib>
#include <sstream>
#include <iostream>
using namespace std;

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
