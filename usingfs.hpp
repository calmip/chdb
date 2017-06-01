
#ifndef USING_FS_H
#define USING_FS_H

//#include <vector>
#include <list>
//#include <stdexcept>
#include <set>
using namespace std;

#include "constypes.hpp"
#include "directories.hpp"

/** 
	\brief This class manages the files contained inside the input or output directory
	       It is used when we work with REAL directories
*/
#include <iostream>
class UsingFs: public Directories {
public:
	UsingFs(const Parameters& p):Directories(p){};

	// consolidateOutput may throw an exception (if incompletly initalized) - Ignore it
	virtual ~UsingFs() {
		try {
			consolidateOutput(true);
		} catch (exception& e){
			//cerr << "Process rank " << rank << " - ";
			//cerr << "EXCEPTION CATCHED DURING UsingFs DESTRUCTOR: \n" << e.what() << '\n';
		}
	}

	//void filesToOutputDb(const vector_of_strings&) {};
	virtual int executeExternalCommand(const vector_of_strings&,const string&,const vector_of_strings&, const string& wd="", const string& sn="");
	void makeOutDir(bool,bool);
	void makeTempOutDir();
	string getTempOutDir() const {
		if(temp_output_dir.length()!=0) return temp_output_dir;
		else throw(logic_error("ERROR - temp_output_dir NOT INITIALIZED"));
	};
	// temporary input diretory not used - But throws if temp not inited for consistency reasons
	// (see Directory::UsingBdbh)
	string getTempInDir() const { 
		if(temp_output_dir.length()!=0) return prms.getInDir();
		else throw(logic_error("ERROR - temporary NOT INITIALIZED"));
	};		
	string getOutDir() const  {
		if(output_dir.length()!=0) return output_dir;
		else throw(logic_error("ERROR - output_dir NOT INITIALIZED"));
	}
	void consolidateOutput(bool from_temp, const string& path="");

//	friend class TestCase1_usingFsfindOrCreateDir_Test;

private:
	void readDir(const string &,size_t) const;
	void readDirRecursive(const string &,size_t,list<Finfo>&,bool) const;
	virtual void findOrCreateDir(const string &);
	virtual void v_readFiles();
	mutable set<string> found_directories;
	string output_dir;
	string temp_output_dir;
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
