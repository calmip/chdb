/**
 * @file   directories.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 11:04:09 2014
 * 
 */

#include <iostream>
#include <fstream>
//#include <iterator>
//#include <set>
using namespace std;

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>
//#include <libgen.h>

//#include "command.h"
#include "usingfs.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "system.hpp"

int operator<(const Finfo& a, const Finfo& b) { return a.st_size < b.st_size; }

/** 
 * @brief set the protected member rank and comm_size
 * 
 * @param r 
 * @param s 
 *
 * @exception the rank and comm_size can be set only 1 time
 */

void Directories::setRank(int r, int s) {
	if (rank>=0) {
		throw(logic_error("ERROR - rank already set"));
	}
	rank=r;
	comm_size=s;
}

/**
 *  @brief Return the nextblok of file names
 *
 *  @pre The vector files should have been initialized
 *  @return A vector of names
 *          An empty vector if there are no more file names
*/

vector_of_strings Directories::nextBlock() {
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
 * @param[in]  p         Path used as a source for the template expansion
 * @param[out] text      String to expand (generally a command line)
 * @param[in]  force_out If true, output directory is forced at begininning of text
 */
void Directories::completeFilePath(const string& p, string& text, bool force_out) {

	// We must work with a readwrite copy !
	// p is the path (a/toto.txt), d the dirname (a), n the name (toto.txt), b the basename (toto), e the ext (txt)
	string d,n,b,e;
	parseFilePath(p,d,n,b,e);

	string id = getTempInDir();
	string od = getTempOutDir();
	static string tmpl1="%in-dir%";
	static string tmpl2="%out-dir%";
	static string tmpl3="%path%";
	static string tmpl4="%basename%";
	static string tmpl5="%name%";
	static string tmpl6="%dirname%";
	
	replaceTmpl(tmpl1,id,text);
	replaceTmpl(tmpl2,od,text);
	replaceTmpl(tmpl3,p,text);
	replaceTmpl(tmpl4,b,text);
	replaceTmpl(tmpl5,n,text);
	replaceTmpl(tmpl6,d,text);
	if (force_out) {
		size_t od_index=text.find(od);
		// if not found, or not found at start
		if (od_index!=0) {
			text = od + '/' + text;
		}
	}
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

/** 
 * @brief Fill if possible the set of files to use
 * 
 */
void Directories::initInputFiles() const {
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
 * @brief Check the name extension versus the required extension (file type)
 * 
 * @param name 
 * 
 * @return true if the type is OK, false if not
 */
bool Directories::isCorrectType(const string & name) const {
	string ext     = '.' + prms.getFileType();
	return isEndingWith(name,ext);
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

void Directories::buildBlocks(list<Finfo>& f_info,vector_of_strings&f) const {
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

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/

