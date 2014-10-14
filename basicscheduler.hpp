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
	BasicScheduler(const Parameters& p, Directories& d):Scheduler(p,d){};
	void mainLoop();
	void errorHandle(ofstream&);

	friend class ChdbTest1_ExecuteCommand_Test;
	friend class ChdbTest1_ExecuteCommandWithErr_Test;
	friend class ChdbTest1_ExecuteCommandFrmList1_Test;
	friend class ChdbTest1_ExecuteCommandFrmList2_Test;
	friend class SchedTestStrInt_readwriteToSndBfr_Test;

private:
	vector_of_int return_values;
	vector_of_strings file_pathes;

	void mainLoopMaster();
	void mainLoopSlave();
	void executeCommand();

	void checkInvariant();

	void allocBfr(void*& bfr, size_t& bfr_sze);
	void writeToSndBfr(void* bfr, size_t, size_t&);
	void readFrmRecvBfr(const void* bfr);
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
