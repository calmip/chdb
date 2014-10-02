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

	
/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/

