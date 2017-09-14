/**
 * @file   scheduler.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Mon Sep 29 14:43:34 2014
 * 
 * @brief  
 * 
 * 
 */

#include <mpi.h>
#include <fstream>
//#include <iterator>
//#include <set>
using namespace std;

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
	
	// @todo - Let the user modify herself the environment ???
	string chdb_env = "CHDB_RANK CHDB_COMM_SIZE";
	if (prms.isVerbose()) {
		setenv("CHDB_VERBOSE","1",true);
		chdb_env += " CHDB_VERBOSE";
	}
	int rvl3 = setenv("CHDB_ENVIRONMENT",chdb_env.c_str(), true);

	if (rvl1!=0 || rvl2!=0 || rvl3!=0)
		throw runtime_error("ERROR - COULD NOT setenv CHDB_RANK, CHDB_COMM_SIZE or CHDB_ENVIRONMENT");

	// Give some infos to dir
	dir.setRank(rank,comm_size); 
}


/**
 * @brief Init the checkList, asking the Directory for the list of files
 * 
 * @pre The directory object must be initialized
 * 
 *****/ 
void Scheduler::_initCheckList() {
	const vector_of_strings& files = dir.getFiles();
	for (vector_of_strings::const_iterator s=files.begin(); s!= files.end(); ++s) {
		checkList[*s] = false;
	}
}

/**
 * @brief Check the items of the list, ie mark the files are "treated" unless they get an error
 * 
 * @pre The ckeckList must be already initialized
 * 
 * @param treated_files A list of treated (= used for computation) files
 * @param return_values A corresponding list of returned values, the file is checked only if value is 0 
 *
 * @exception Throw a logic_error if some file is not in the list
 * 
 *********/
void Scheduler::_checkListItems(const vector_of_strings& treated_files, const vector_of_int& return_values) {
	for (size_t i=0; i < treated_files.size(); ++i) {
		string f = treated_files[i];
		int    v = return_values[i];
		if (checkList.find(f) == checkList.end()) {
			string msg = "ERROR - THE FILE " + f + " IS NOT IN THE CHECK LIST !";
			throw logic_error(msg.c_str());
		}
		if (v==0) checkList[f] = true;
	}
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

/****
 * @brief Called by main to inform the scheduler that a signal was received !
 *        See the SignalHandle class
 *        If master, Save the state, wait 25 s and exit
 *        If slave, wait 25 s and exit
 * 
 * @param signal The signal received
 * 
 * @return If master, DOES NOT RETURN (call _exit)
 * 
 *****/
void Scheduler::SetSignal(int signal) {
	if (isMaster()) {
        cerr << "Scheduler rank=" << getRank() << " received a signal " << signal << " - Creating CHDB-INTERRUPTION.txt and exiting" << endl;
		ofstream ofs ("CHDB-INTERRUPTION.txt", ofstream::out);
		ofs << "# CHDB WAS INTERRUPTED - You may restart chdb using this file with the switch --in-files\n";
		ofs << dir.howToConsolidate() << endl;
		
		int j = 0;
		for (map<string,bool>::iterator i = checkList.begin(); i != checkList.end(); ++i) {
			if ( i->second == false) {
				j++;
				ofs << i->first << endl;
			}
		}
		
		ofs << "# Number of files not yet processed = " << j << endl;
		ofs.close();
		
		// Close open files, if necessary
		if (err_file.is_open())    err_file.close();
		if (report_file.is_open()) report_file.close();
	}
	else {
		cerr << "Scheduler rank=" << getRank() << " received a signal - " << signal << " - Sleeping 25 s" << endl;
	}
	sleep(25);
	_exit(0);
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

