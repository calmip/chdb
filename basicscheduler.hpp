/**
 * @file   basicscheduler.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Tue Sep 30 09:52:07 2014
 * 
 * @brief  This class implements a basic scheduling strategy: individual jobs
 *         are launched one after another, as soon as a slave becomes available we send a new job.
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
 */

#ifndef BASIC_SCHEDULER_H
#define BASIC_SCHEDULER_H

using namespace std;

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

