/**
 * This file is part of chdb software
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2022    C A L M I P
 *  chdb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with chdb.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <iostream>

#if __cplusplus >= 201703L
    #include <filesystem>
#endif

using namespace std;

#include <libgen.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>

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
 * @exception SigChildExc if the child received a signal, runtime_error if child returned with a status != 0 and err_flg is true
 * @note If the calling process (ie the chdb slave) receives a SIGINT signal (interruption), this signal will be IGNORED (see the function system())
 *
 */

//#define DEBUG_SYSTEM 1

int callSystem(string cmd, bool err_flg) {
#ifdef DEBUG_SYSTEM
    cerr << "DEBUG - CALLING callSystem cmd=" << cmd << '\n';
#endif

    int retry = NUMBER_OF_RETRIES;
    bool should_retry;
    int sts  = 0;              // The return of the system function
    int csts = 0;              // The return code of the command (if exited correctly)
    do {
        sts = system(cmd.c_str());
        
        // if the child received a signal, throw a SigChildExc
        if (WIFSIGNALED(sts)) {
            int sig = WTERMSIG(sts);
            throw(SigChildExc(sig));
        }
        
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
                msg += "could not execute cmd - may be a shared library not found ?";
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

    if (WIFEXITED(sts) && csts!=0 && err_flg) {
        ostringstream err;
        err << "ERROR ! " << cmd << " returned an error: " << csts;
        throw(runtime_error(err.str()));
    }
    
    // Child returned normally (warning ! not tested !) => return the command return code
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
#if __cplusplus >= 201703L
    #warning Using C++17 filesystem library
    d = filesystem::current_path().string();
#else
    #warning NOT USING C++17 filesystem library
    const char* d_c = get_current_dir_name();
    d = (string) d_c;
    free((void*)d_c);
#endif
}

/** 
 * @brief Sleep duration, counted in milliseconds
 * 
 * @param duration
 */
void sleepMs(unsigned int duration) {
    struct timespec req;
    if (duration > 1000)
    {
        req.tv_sec  = duration / 1000;	// 2500 => 2
        req.tv_nsec = 1000000 * (duration % 1000); // 1 ms = 1000000 ns, 2500 % 1000 = 500
    } 
    else
    {
        req.tv_sec  = 0;
        req.tv_nsec = 1000000*duration; // 1 ms = 1000000 ns
    }
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

#if __cplusplus >= 201703L
    #warning Using C++17 filesystem library
    filesystem::path fs_path(path);
    dir = fs_path.parent_path().string();
    if (dir == "") dir = ".";   // Current directory

    name = fs_path.filename().string();
    if (name[0] == '.')
    {
        base = "";
        ext = name;
        ext.erase(0,1);
    }
    else
    {
        base = fs_path.stem().string();
        ext = fs_path.extension().string();
    }
    if (ext != "" && ext[0] == '.') ext.erase(0,1);// .txt --> txt
#else
    #warning NOT USING C++17 filesystem library
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
#endif

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
 * @brief Returns true if string ends with the suffix
 * 
 * @param name 
 * @param suffix 
 * 
 * @return 
 */
bool isEndingWith(const string& name, const string& suffix) {
#if __cplusplus > 201703L
    #warning Using C++20 ends_with
    return name.ends_with(suffix);
#else
    size_t suffix_len = suffix.length();
    size_t name_len = name.length();
    if ( suffix_len < name_len ) {
        string name_suffix = name.substr(name_len-suffix_len);
        return ( name_suffix == suffix );
    } else {
        return false;
    }
#endif
}

/** 
 * @brief Returns true if string begins with the heading
 * 
 * @param string
 * @param prefix
 * 
 * @return 
 */
bool isBeginningWith(const string& name, const string& prefix) {
#if __cplusplus > 201703L
    #warning Using C++20 starts_with
    return name.starts_with(prefix);
#else    
    size_t prefix_len = prefix.length();
    size_t nme_len = name.length();
    if ( prefix_len <= nme_len ) {
        string nme_head = name.substr(0,prefix_len);
        return ( nme_head == prefix );
    } else {
        return false;
    }
#endif
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
 *        Storing three files: 0003xxx\0xxx\0xxx\0
 *        Storing Zero file: 0000
 * 
 * @param[in] files_names A vector of file names
 * @param[in] bfr The buffer (should be already allocated)
 * @param[in] bfr_size The buffer size
 * @param[out] data_size The length of data stored
 * @exception A runtime_exception is thrown if the buffer is too small
 * 
 */
void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size ) {
    size_t local_data_size=sizeof(int);
    for (size_t i=0; i<file_pathes.size(); ++i) {
        local_data_size += file_pathes[i].length()+1;
    }

    // data_size unchanged if there is an exception
    if (local_data_size > bfr_size) {
        throw(runtime_error("ERROR - Buffer too small"));
    }
    data_size = local_data_size;
    
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

