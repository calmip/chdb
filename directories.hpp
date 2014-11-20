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
#include <list>
//#include <stdexcept>
#include <algorithm>
using namespace std;


//#include "SimpleOpt.h"
#include "constypes.hpp"
#include "parameters.hpp"

// The size max of a file path
#define FILEPATH_MAXLENGTH 2000

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
	Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};

class Directories: private NonCopyable {

public:
	Directories(const Parameters& p):prms(p),rank(-1),comm_size(0),blk_ptr(files.begin()){};
	virtual ~Directories(){};
	void setRank(int,int);

	virtual void   makeOutDir(bool,bool) = 0;
	virtual void   makeTempOutDir()  = 0;
	virtual string getOutDir() const = 0;
	virtual string getTempOutDir() const = 0;
	virtual void findOrCreateDir(const string &) const = 0;
	virtual void buildBlocks(list<Finfo>&, vector_of_strings&) const = 0;

	virtual void consolidateOutput(bool from_temp, const string& path="") const = 0;

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
		files_size = count_if(files.begin(), files.end(), isNotNullStr);
		blk_ptr=files.begin();
	}

	vector_of_strings nextBlock();
	void completeFilePath(const string& p, string& text, bool force_out=false);
	virtual int executeExternalCommand(const string&,const vector_of_strings&) const = 0;
	size_t getNbOfFiles() { readFiles(); return files_size; };
	//void insertOutFilesToDb(const vector_of_strings&) {};

	friend class TestCase1_block1_Test;
	friend class TestCase1_usingFsfindOrCreateDir_Test;

protected:
	const Parameters& prms;
	mutable vector_of_strings files;
	mutable size_t files_size;
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
