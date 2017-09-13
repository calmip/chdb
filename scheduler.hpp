/**
 * @file   schedulers.hpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 12:25:30 2014
 * 
 * @brief  
 * 
 * 
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

//#include <vector>
//#include <string>
//#include <stdexcept>
#include <csignal>

#include <iostream>
#include <fstream>
#include <map>
using namespace std;
//#include <cassert>

//#include "gtest/gtest.h"

//#include "SimpleOpt.h"
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

	//static void sig_handler(int signal);
	//static int GetSignal() {return signal_received;};
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
	//int signal_received;
	
	map<string,bool> checkList;
	
	// The address of the ONLY Scheduler object (as it is non copyable), useful for sig_hanlder
	//static Scheduler* sched;

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
