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
	Directories(const Parameters& p):prms(p),rank(-1),comm_size(-1),blk_ptr(files.begin()){};
	void setRank(int,int);

	virtual void makeOutputDir() const = 0;

//	void readFiles() { getFiles(); };
//	const vector_of_strings& getFiles() { 
//		const vector_of_strings& rvl = v_readFiles();
//		blk_ptr=files.begin();
//		return rvl;
//	};
	const vector_of_strings& getFiles() {
		readFiles();
		return files;
	}
	void readFiles() {
		v_readFiles();
		blk_ptr=files.begin();
	}

	vector_of_strings nextBlock();
	void completeFilePath(const string& p, string& command);
	virtual int executeExternalCommand(const string&,const vector_of_strings&) const = 0;
	size_t getNbOfFiles() { readFiles(); return files.size(); };
	//void insertOutFilesToDb(const vector_of_strings&) {};

protected:
	const Parameters& prms;
	mutable vector_of_strings files;
	int rank;
	int comm_size;

private:
	mutable vector_of_strings::iterator blk_ptr;
	void replaceTmpl(const string& tmpl, const string& value, string& text);
	virtual void v_readFiles() = 0;
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
