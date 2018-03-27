/**
 * @file   usingbdbh.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * 
 * @This class manages the files contained inside the input or output directory
 *       It is used when we work with bdbh databases
 *       WARNING - BDBH DATABASES IS STILL EXPERIMENTAL
 * 
 * This file is part of chdb software
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2018 Emmanuel Courcelle
 *  chdb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with chdb.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 * 
 */
 
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

	// Explaining how to consolidate data manually
	string howToConsolidate() const;

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
	bool signal_received;
	void checkParameters();

};

#endif
