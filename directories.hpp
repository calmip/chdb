/**
 * @file   directories.hpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 11:03:01 2014
 * 
 * @brief  This class manages the files contained inside the input or output directory
 *         It has several subclasses, for managing the different types of directories 
 *         (real directory, bdbh file, etc)
 * 
 * 
 */

#ifndef DIRECTORIES_H
#define DIRECTORIES_H

//#include <vector>
//#include <string>
//#include <stdexcept>
using namespace std;


//#include "SimpleOpt.h"
#include "constypes.hpp"
#include "parameters.hpp"

// The size max of a file path
#define FILEPATH_MAXLENGTH 2000

class Directories: private NonCopyable {

public:
	Directories(const Parameters& p):prms(p){};

	virtual const vector_of_strings& getFiles() const = 0;
	vector_of_strings nextBlock();
	void completeFilePath(const string& p, string& command);
	void insertOutFilesToDb(const vector_of_strings&) {};

protected:
	const Parameters& prms;
	mutable vector_of_strings files;

private:
	vector_of_strings::iterator blk_ptr;
	void replaceTmpl(const string& tmpl, const string& value, string& text);
};

#endif

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/
