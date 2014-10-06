
#ifndef USING_FS_H
#define USING_FS_H

//#include <vector>
//#include <string>
//#include <stdexcept>
#include <set>
using namespace std;


#include "constypes.hpp"
#include "directories.hpp"

/** 
	\brief This class manages the files contained inside the input or output directory
	       It is used when we work with REAL directories
*/

class UsingFs: public Directories {
public:
	UsingFs(const Parameters& p):Directories(p){};

	virtual const vector_of_strings& getFiles() const;
	void filesToOutputDb(const vector_of_strings&) {};
	int executeExternalCommand(const string&,const vector_of_strings&) const;
	void makeOutputDir() const;

	friend class ChdbTest_usingFsfindOrCreateDir_Test;

private:
	void readDir(const string &,size_t) const;
	void findOrCreateDir(const string &) const;
	bool isCorrectType(const string &) const;
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
