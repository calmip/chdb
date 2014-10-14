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
using namespace std;


//#include "gtest/gtest.h"

//#include "SimpleOpt.h"
#include "constypes.hpp"
#include "parameters.hpp"
#include "directories.hpp"

class Scheduler: private NonCopyable {

public:

	Scheduler(const Parameters& p, Directories& d);
	int getRank() const     { return rank; };
	int getCommSize() const { return comm_size; };
	bool isMaster() const   { return rank==0;};

	static void init(int,char**);
	static void finalize();
	static void abort();

	virtual void mainLoop()=0;
	virtual void errorHandle(ofstream&)=0;
	void startTimer();
	double getTimer();

	// for gtests
	friend class SchedTestStr_vctToBfrStrings_Test;
	friend class SchedTestStr_bfrToVctStrings_Test;
	friend class SchedTestInt_vctToBfrInt_Test;
	friend class SchedTestInt_bfrToVctInt_Test;

protected:
	void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size);
	void bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& files_names);
	void vctToBfr(const vector_of_int& values, void* bfr, size_t bfr_size, size_t& data_size);
	void bfrToVct(void const* bfr, size_t& data_size, vector_of_int& values);

	const Parameters& prms;
	Directories& dir;

	int comm_size;
	int rank;

private:
	double start_time;

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
