
#ifndef USING_FS_H
#define USING_FS_H

//#include <vector>
#include <list>
//#include <stdexcept>
#include <set>
using namespace std;

#include "constypes.hpp"
#include "directories.hpp"

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
	Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};

/** 
	\brief This class manages the files contained inside the input or output directory
	       It is used when we work with REAL directories
*/
class UsingFs: public Directories {
public:
	UsingFs(const Parameters& p):Directories(p){};

	void filesToOutputDb(const vector_of_strings&) {};
	int executeExternalCommand(const string&,const vector_of_strings&) const;
	void makeOutputDir() const;
	void buildBlocks(list<Finfo>&, vector_of_strings&) const;

	friend class ChdbTest_usingFsfindOrCreateDir_Test;

private:
	void readDir(const string &,size_t) const;
	void readDirRecursive(const string &,size_t,list<Finfo>&,bool) const;
	void findOrCreateDir(const string &) const;
	bool isCorrectType(const string &) const;
	void initInputFiles() const;
	virtual void v_readFiles();

	mutable set<string> input_files;
	mutable set<string> found_directories;
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
