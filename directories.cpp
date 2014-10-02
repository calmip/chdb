/**
 * @file   directories.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 11:04:09 2014
 * 
 */

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
#include <libgen.h>

//#include "command.h"
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

/**
 *  @brief Return the nextblok of file names
 *
 *  @return A vector of names
 *          An empty vector if there are no more file names
*/

vector_of_strings Directories::nextBlock() {
//	getFiles();
	if (files.empty()) {
		getFiles();
		blk_ptr = files.begin();
//		throw(runtime_error("ERROR - No files in the input directory"));
	}

	vector_of_strings blk;
	if ( blk_ptr != files.end() ) {

		int blk_size = prms.getBlockSize();
		vector_of_strings::iterator blk_next_ptr;
		if (files.end()-blk_ptr > blk_size) {
			blk_next_ptr = blk_ptr + blk_size;
		} else {
			blk_next_ptr = files.end();
		}
		blk.assign(blk_ptr,blk_next_ptr);
		blk_ptr = blk_next_ptr;
	}

	return blk;
}

/** 
 * @brief Replace some templates in the parameter, using the file_path:
 *        file pathes: #path#, #dirname#
 * 
 * @param[in]    p Path used as a source for the template expansion
 * @param[out] command String to expand (generally a command line)
 */
void Directories::completeFilePath(const string& p, string& command) {

	// We must work with a readwrite copy !
	// p is the path (a/toto.txt), d the dirname (a), n the name (toto.txt), b the basename (toto)
	char*  file_path = (char*) malloc(p.length()+1);
	strcpy(file_path,p.c_str());
	string d = dirname(file_path);
	strcpy(file_path,p.c_str());
	string n = basename(file_path);
	free(file_path);

	string b;
	if (n[0] != '.') {
		size_t dot = n.find_last_of('.');
		b = (dot!=string::npos)?n.substr(0,dot):b;
	} else {
		b = "";
	}

	static string tmpl1="#path#";
	static string tmpl2="#basename#";
	static string tmpl3="#name#";
	static string tmpl4="#dirname#";
	replaceTmpl(tmpl1,p,command);
	replaceTmpl(tmpl2,b,command);
	replaceTmpl(tmpl3,n,command);
	replaceTmpl(tmpl4,d,command);
}

/** 
 * @brief replace a template with value
 * 
 * @param[in]  tmpl  The template to look for in text
 * @param[in]  value The value to replace with 
 * @param[out] text
 */
void Directories::replaceTmpl(const string& tmpl, const string& value, string& text) {
	size_t pos = 0;
	do {
		pos = text.find(tmpl,pos);
		if (pos != string::npos) {
			text.replace(pos,tmpl.length(),value);
		}
	} while(pos != string::npos);
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

