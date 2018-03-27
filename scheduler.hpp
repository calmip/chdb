/**
 * @file   schedulers.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Mon Sep 29 12:25:30 2014
 * 
 * @brief  This abstract class is the base classe for all scheduler-type classes
 * 
 * 
 * This file is part of chdb software
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2018    C A L M I P
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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <csignal>

#include <iostream>
#include <fstream>
#include <map>
using namespace std;

//#include "gtest/gtest.h"

#include "constypes.hpp"
#include "parameters.hpp"
#include "directories.hpp"

class Scheduler: private NonCopyable {
public:
	Scheduler(const Parameters& p, Directories& d);
	int getRank() const     { return rank; };
	size_t getNbOfSlaves() const { return comm_size-1; };
	bool isMaster() const   { return rank==0;};

	static void init(int,char**);
	static void finalize();
	static void abort();

	virtual void mainLoop()=0;
	virtual bool errorHandle()=0;
	virtual size_t getTreatedFiles() const=0;

	void startTimer();
	double getTimer();

	void SetSignal(int s);// { signal_received = s; };

	// for gtests
	//friend class SchedTestStr_vctToBfrStrings_Test;
	//friend class SchedTestStr_bfrToVctStrings_Test;
	//friend class SchedTestInt_vctToBfrInt_Test;
	//friend class SchedTestInt_bfrToVctInt_Test;

protected:

	void _initCheckList();
	void _checkListItems(const vector_of_strings&, const vector_of_int&);
	
	const Parameters& prms;
	Directories& dir;

	int comm_size;
	int rank;

	// Derived classes should know what to do with those files
	ofstream err_file;
	ofstream report_file;

private:
	double start_time;
	
	map<string,bool> checkList;
	
};

#endif
