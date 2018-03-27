/**
 * @file   usingfs.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * 
 * @This class manages the files contained inside the input or output directory
 *       It is used when we work with filesystem stored files (ie NOT in bdbh databases)
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

	// Explaining how to consolidate data manually
	string howToConsolidate() const { string out="# \n"; return out;};

	//void filesToOutputDb(const vector_of_strings&) {};
	virtual int executeExternalCommand(const vector_of_strings&,const string&,const vector_of_strings&, const string& wd="", const string& sn="");
	void makeOutDir(bool,bool);
	void makeTempOutDir();
	string getTempOutDir() const {
		if (temp_output_dir.length()==0 && !prms.isTypeDir()) throw(logic_error("ERROR - temp_output_dir NOT INITIALIZED"));
		return temp_output_dir;
	};

	// temporary input directory not used - But throws if temp not inited for consistency reasons
	// (see Directory::UsingBdbh)
	string getTempInDir() const { 
		if (temp_output_dir.length()==0 && !prms.isTypeDir()) throw(logic_error("ERROR - temp_output_dir NOT INITIALIZED"));
		return prms.getInDir();
	};		

	string getOutDir() const  {
		if(output_dir.length()==0 && prms.isTypeFile()) throw(logic_error("ERROR - output_dir NOT INITIALIZED"));
		return output_dir;
	}
	void consolidateOutput(bool from_temp, const string& path="");

//	friend class TestCase1_usingFsfindOrCreateDir_Test;

private:
	void readDir(const string &,size_t) const;
	void readDirRecursive(const string &,size_t,list<Finfo>&,bool) const;
	virtual void findOrCreateDir(const string &);
	virtual void v_readFiles();
	mutable set<string> found_directories;
	void checkParameters();
	
	string output_dir;
	string temp_output_dir;
};

#endif
