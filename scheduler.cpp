/**
 * @file   scheduler.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 14:43:34 2014
 * 
 * @brief  
 * 
 * 
 */

//#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <mpi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>

//#include "command.h"
#include "scheduler.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

/** 
 * @brief Build a Scheduler object
 *        throw an error if MPI not yet initialized or if no slave 
 *        Set the environment variables CHDB_RANK and CHDB_COMM_SIZE
 * 
 * @param p 
 * @param d 
 * 
 * @return 
 */

Scheduler::Scheduler(const Parameters& p, Directories& d) : prms(p),dir(d),start_time(-1) {
	int flg;
	MPI_Initialized(&flg);
	if (flg==0) {
		throw logic_error("ERROR - MPI not yet initialized");
	};
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &comm_size);

	if (comm_size==1) {
		throw logic_error("ERROR - YOU SHOULD HAVE AT LEAST 1 SLAVE");
	}

	// Set two env variables
	ostringstream os_tmp;
	os_tmp << rank;
	int rvl1 = setenv("CHDB_RANK",os_tmp.str().c_str(),true);
	os_tmp.str("");

	os_tmp << comm_size;
	int rvl2 = setenv("CHDB_COMM_SIZE",os_tmp.str().c_str(),true);

	if (rvl1!=0 || rvl2!=0)
		throw runtime_error("ERROR - COULD NOT setenv CHDB_RANK or CHD_COMM_SIZE");

	// Give some infos to dir
	dir.setRank(rank,comm_size); 
}

/** 
 * @brief Call MPI_Abort
 * 
 */
void Scheduler::abort() {
	MPI_Abort(MPI_COMM_WORLD,1);
}

/** 
 * @brief call MPI_init 
 * 
 * @param argc 
 * @param argv 
 */

void Scheduler::init(int argc, char**argv) {
	MPI_Init(&argc,&argv);
}
	
void Scheduler::startTimer() {
	start_time = MPI_Wtime();
}

double Scheduler::getTimer() {
	if (start_time==-1) {
		throw(logic_error("ERROR - Timer was not started !"));
	}
	return MPI_Wtime()-start_time;
}

/** 
 * @brief Send a barrier to be sure everybody is synchronized, then finalize
 * 
 */
void Scheduler::finalize() {
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/

