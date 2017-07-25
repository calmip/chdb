/**
 * @file   basicscheduler.hpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Tue Sep 30 09:52:07 2014
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASIC_SCHEDULER_H
#define BASIC_SCHEDULER_H

//#include <vector>
//#include <string>
//#include <stdexcept>
using namespace std;


//#include "SimpleOpt.h"
#include "constypes.hpp"
#include "scheduler.hpp"

class BasicScheduler: public Scheduler {
public:
	BasicScheduler(const Parameters& p, Directories& d):Scheduler(p,d),first_execution(true),treated_files(0){};
	void mainLoop();
	bool errorHandle();
	size_t getTreatedFiles() const { return treated_files; };
 
	friend class TestCase1_ExecuteCommandFrmList1_Test;
	friend class TestCase1_ExecuteCommandFrmList2_Test;
	friend class SchedTestStrInt_readwriteToSndBfr_Test;
//	friend class WithOrNoTmp_AbortOnError_Test;
//	friend class WithOrNoTmp_ContinueOnError_Test;
	friend class TestCase1_AbortOnError_Test;
	friend class TestCase1_ContinueOnError_Test;

private:
	vector_of_int return_values;        // The return value of each execution of the command - maintained by the slaves
	vector_of_double wall_times;        // The wall_time of each command, used only with the option --report - maintained by the slaves
	vector_of_double wall_time_slaves;  // The cumulated elapsed time of each slave - maintained by the master
	vector_of_int files_slaves;         // Number of files treated by each slave ( maintained by the master
	vector_of_strings file_pathes;      // The file_pathes to be treated at next iteration (ie: the Block)

	void mainLoopMaster();
	void mainLoopProlog();
	void mainLoopSlave();
	void sendEndMsg(void*, int);


	void executeCommand();
	bool first_execution;

	void checkInvariant();

	void allocBfr(void*& bfr, size_t& bfr_sze);
	void writeToSndBfr(void* bfr, size_t, size_t&);
	void readFrmRecvBfr(const void* bfr);

	void reportHeader(ostream &);
	void reportBody(ostream&, int);
	void reportSummary(ostream&);

	void openErrFileIfNecessary();
	void openReportFileIfNecessary();
	
	size_t treated_files;
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
