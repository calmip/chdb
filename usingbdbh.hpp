#ifndef USING_BDBH_H
#define USING_BDBH_H

//#include <vector>
#include <list>
//#include <stdexcept>
//#include <set>
#include <memory>
using namespace std;

#include "constypes.hpp"
#include "directories.hpp"
#include "bdbh/command.hpp"

#define DEFAULT_BDBH_TMP_DIRECTORY "."

typedef auto_ptr<bdbh::BerkeleyDb> BerkeleyDb_aptr;

/** 
	\brief This class manages the files contained inside the input or output directory
	       It is used when we work with bdbh databases
*/
class UsingBdbh: public Directories {
public:
	UsingBdbh(const Parameters& p);
	virtual ~UsingBdbh();

	//void filesToOutputDb(const vector_of_strings&) {};
	int executeExternalCommand(const vector_of_strings&,const string&,const vector_of_strings&, const string& wd="", const string& sn="");
	void makeTempOutDir();
	void makeOutDir(bool,bool);
	string getTempOutDir() const {
		if(temp_output_dir.length()!=0) return temp_output_dir;
		else throw(logic_error("ERROR - temp_output_dir NOT INITIALIZED"));
	};
	string getTempInDir() const {
		if(temp_input_dir.length()!=0) return temp_input_dir;
		else throw(logic_error("ERROR - temp_input_dir NOT INITIALIZED"));
	};
	string getTempDbDir() const {
		if(temp_db_dir.length()!=0) return temp_db_dir;
		else throw(logic_error("ERROR - temp_db_dir NOT INITIALIZED"));
	};
	string getOutDir() const  {
		if(output_dir.length()!=0) return output_dir;
		else throw(logic_error("ERROR - output_dir NOT INITIALIZED"));
	};
	void consolidateOutput(bool from_temp, const string& path="");

	virtual void SetSignal(int signal);
	virtual void Sync();
	
//	friend class TestCase1_usingFsfindOrCreateDir_Test;

private:
	BerkeleyDb_aptr input_bdb;    // The Input database
	BerkeleyDb_aptr output_bdb;   // The Output database
	BerkeleyDb_aptr temp_bdb;     // The Temp database
	virtual void findOrCreateDir(const string &);
	virtual void v_readFiles();
	// mutable set<string> input_files;
	set<string> found_directories;
	string output_dir;
	string temp_dir;
	string temp_output_dir;
	string temp_input_dir;
	string temp_db_dir;
	mutable bool need_consolidation;
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
