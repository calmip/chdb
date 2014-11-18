/**
 * @file   basicscheduler.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Tue Sep 30 09:57:49 2014
 * 
 * @brief  
 * 
 * 
 */

#include <mpi.h>
#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <algorithm>
#include <cmath>
//#include <cstdlib>
#include <cerrno>

//#include "command.h"
#include "system.hpp"
#include "basicscheduler.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

// The tags MPI defined by CHDB
#define CHDB_TAG_READY 1000
#define CHDB_TAG_GO    1010
#define CHDB_TAG_END   1020

void BasicScheduler::mainLoop() {

// May be useful for debugging with gdb
/*	
	{
	pid_t pid = getpid();
	ostringstream s_tmp;
	s_tmp << "gdbserver host:2345 --attach " << pid << "&";
	cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
	system(s_tmp.str().c_str());
	sleep(5);
	}
*/

	// Used only by the master
	ofstream err_file;
	ofstream report_file;

	// some initialization specific to the master
	if (isMaster()) {
		mainLoopProlog(err_file,report_file);
	}

	// A barrier to wait for slave initialization
	MPI_Barrier(MPI_COMM_WORLD);

	// Enter the loop
	if (isMaster()) {
		mainLoopMaster(err_file,report_file);
	} else {
		mainLoopSlave();
	}
}

void BasicScheduler::mainLoopProlog(ofstream& err_file, ofstream& report_file) {

	// read the file names
	dir.readFiles();

	// check the number of blocks versus number of slaves and throw an exception if too many slaves
	size_t slaves_max = dir.getNbOfFiles()/prms.getBlockSize();
	if (dir.getNbOfFiles() % prms.getBlockSize() != 0) {
		slaves_max += 1;
	};

	if (slaves_max<getNbOfSlaves()) {
		ostringstream out;
		out << "ERROR - You should NOT use more than " << slaves_max << " slaves";
		throw(runtime_error(out.str()));
	}

	// Create the output directory
	dir.makeOutDir(false,false);

	// Open the error file, if any
	if (!prms.isAbrtOnErr()) {
		string err_name = prms.getErrFile();
		err_file.open(err_name.c_str());
		if (!err_file.good()) {
			string msg = "ERROR - File could not be opened: ";
			msg += err_name;
			throw(runtime_error(msg));
		}
	}

	// Open the report file, if any
	if (prms.isReportMode()) {
		string report_name = prms.getReport();
		report_file.open(report_name.c_str());
		if (!report_file.good()) {
			string msg = "ERROR - File could not be opened: ";
			msg += report_name;
			throw(runtime_error(msg));
		} else {
			reportHeader(report_file);
		}
	}

	return_values.clear();
	wall_times.clear();
}

/** 
 * @brief Send to the slave a message with no data and END TAG
 * 
 * @param slave 
 */

void BasicScheduler::sendEndMsg(void* send_bfr, int slave) {
	// Send an END message to the slave
	MPI_Send(send_bfr, 0, MPI_BYTE, slave, CHDB_TAG_END, MPI_COMM_WORLD);
}

/** 
 * @brief The main loop for the master
 * 
 */
void BasicScheduler::mainLoopMaster(ofstream& err_file, ofstream& report_file) {
	MPI_Status sts;

	// loop over the file blocks
	// Listen to the slaves
	size_t bfr_size=0;
	void* send_bfr=NULL;
	void* recv_bfr=NULL;

	allocBfr(send_bfr,bfr_size);
	allocBfr(recv_bfr,bfr_size);

	file_pathes = dir.nextBlock();

	while(!file_pathes.empty()) {

		// Prepare the send buffer for the next message
		size_t send_msg_len=0;
		writeToSndBfr(send_bfr,bfr_size,send_msg_len);
		
		// Listen to the slaves
		MPI_Recv((char*)recv_bfr,(int)bfr_size, MPI_BYTE, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
		int talking_slave = sts.MPI_SOURCE;
		//size_t recv_msg_len;
		//MPI_Get_count(&sts, MPI_BYTE, (int*) &recv_msg_len);

        // Init return_values, may be wall_times, and file_pathes with the message
		readFrmRecvBfr(recv_bfr);
		
		// counting the files
		treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);

		// Write info to report
		if (prms.isReportMode()) {
			reportBody(report_file, talking_slave);
		}

		// Handle the error, if error found and abort mode exit from the loop
		bool err_found = errorHandle(err_file);
		if (err_found && prms.isAbrtOnErr()) {
			sendEndMsg(send_bfr, talking_slave);
			break;

		}

		// Send the block to the slave
		MPI_Send(send_bfr,send_msg_len,MPI_BYTE,talking_slave,CHDB_TAG_GO,MPI_COMM_WORLD);

		// Init return_values and file_pathes for next iteration
		return_values.clear();
		wall_times.clear();
		file_pathes = dir.nextBlock();
	}

	// loop over the slaves: when each slave is ready, send him a msg END
	//                       and when the slave sends back a tag END, consolidate his work and forget him
	int working_slaves = getNbOfSlaves(); // The master is not a slave
	while(working_slaves>0) {
		MPI_Recv(recv_bfr, bfr_size, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &sts);
		int talking_slave = sts.MPI_SOURCE;
		int tag = sts.MPI_TAG;

		// Init return_values and file_pathes with the message
		readFrmRecvBfr(recv_bfr);
		
		// We received a tag "READY": send the message END
		if (tag==CHDB_TAG_READY) {

			// counting the files
			treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);
			
			// Handle the error
			errorHandle(err_file);
		
			// Write info to report
			if (prms.isReportMode()) {
				reportBody(report_file, talking_slave);
			}

			// Send an empty message tagged END to the slave
			sendEndMsg(send_bfr, talking_slave);
		}

		// We received a tag "END": consolidate data (if necessary) and forget this slave
		else {
			dir.consolidateOutput(false,file_pathes[0]);
			working_slaves--;
		}
	}

	// close err_file
	if (err_file.is_open()) err_file.close();

	// write report summary
	if (prms.isReportMode()) {
		reportSummary(report_file);
	}

	// free memory
	free(send_bfr);
	free(recv_bfr);
}

/** 
 * @brief Write the header in the report file
 *        Assign the vector wall_time_slaves
 * 
 * @param os 
 */
void BasicScheduler::reportHeader(ostream& os) {
	os << "SLAVE\tTIME(s)\tSTATUS\tINPUT PATH\n";

	wall_time_slaves.clear();
	wall_time_slaves.assign(getNbOfSlaves()+1,0.0);
	files_slaves.assign(getNbOfSlaves()+1,0);
}

/** 
 * @brief Write some report lines
 *        Update the corresponding cell of wall_time_slaves
 * 
 * @param os 
 * @param rank 
 */
void BasicScheduler::reportBody(ostream& os, int rank) {
	double wall_time_slave=0;
	int n=0;
	for (size_t i=0; i<file_pathes.size(); ++i) {
		if (file_pathes[i].size()!=0) {
			os << rank << '\t';
			os << wall_times[i] << '\t';
			os << return_values[i] << '\t';
			os << file_pathes[i] << '\n';
			wall_time_slave += wall_times[i];
			n += 1;
		}
	}
	wall_time_slaves[rank] += wall_time_slave;
	files_slaves[rank]     += n;
}

/** 
 * @brief Write the wall_time of each slave
 * 
 * @param os 
 */
void BasicScheduler::reportSummary(ostream& os) {
	os << "----------------------------------------------\n";
	os << "SLAVE\tN INP\tCUMULATED TIME (s)\n";
	size_t nb_slaves = getNbOfSlaves();
	double min=1E6;
	double max=0;
	double avg=0;
	double std=0;
	for (size_t i=1; i<=nb_slaves; ++i) {
		if (wall_time_slaves[i]<min) min=wall_time_slaves[i];
		if (wall_time_slaves[i]>max) max=wall_time_slaves[i];
		avg += wall_time_slaves[i];
		os << i << '\t' << files_slaves[i] << '\t' << wall_time_slaves[i] << '\n';
	}

	// computing the average time and standard deviation
	avg /= nb_slaves;
	std =  0;
	if (nb_slaves>1) {
		for (size_t i=1;i<=nb_slaves; ++i) {
			std += (wall_time_slaves[i]-avg)*(wall_time_slaves[i]-avg);
		}
		std = sqrt(std/(nb_slaves-1));
	}

	os << "----------------------------------------------\n";
	os << "AVERAGE TIME (s)        = " << avg << '\n';
	os << "STANDARD DEVIATION (s)  = " << std << '\n';
	os << "MIN VALUE (s)           = " << min << '\n';
	os << "MAX VALUE (s)           = " << max << '\n';
}
	
/** 
 * @brief The main loop for the slaves
 * 
 */
void BasicScheduler::mainLoopSlave() {
	vector_of_strings blk;
	MPI_Status sts;

	size_t bfr_size=0;
	void* bfr=NULL;
	allocBfr(bfr,bfr_size);
/*
	{
		pid_t pid = getpid();
		ostringstream s_tmp;
		s_tmp << "gdbserver host:2345 --attach " << pid << "&";
		cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
		system(s_tmp.str().c_str());
		sleep(5);
	}
*/
	// all msgs are sent/received to/from the master
	const int master    = 0;
	int tag             = CHDB_TAG_GO;
	size_t send_msg_len = 0;
	while(tag==CHDB_TAG_GO) {

		// Prepare the send buffer for the next message
		writeToSndBfr(bfr,bfr_size,send_msg_len);

		// Send the report+ready message to the master, receive a list of files to treat
		MPI_Sendrecv_replace((char*)bfr,(int)bfr_size,MPI_BYTE,master,CHDB_TAG_READY,master,MPI_ANY_TAG,MPI_COMM_WORLD,&sts);
		tag = sts.MPI_TAG;

        // Init file_pathes with the message
		readFrmRecvBfr(bfr);

		if (tag==CHDB_TAG_GO) {
			executeCommand();
		}
	}

	// END tag received: consolidate output directory from temporary and leave
	// If first_execution:
	//    - The slave did nothing !
	//    - The temporary diectory is NOT initialized
	//    ---> There is nothing to consolidate, and calling consolidateOutput will throw an exception
	if (!first_execution) {
		dir.consolidateOutput(true);
	}

	// Send a last message to the master: tag END, name of consolidated output directory
	file_pathes.clear();
	return_values.clear();
	wall_times.clear();
	if (!first_execution) {
		file_pathes.push_back(dir.getOutDir());
	}

	writeToSndBfr(bfr,bfr_size,send_msg_len);
	MPI_Send(bfr, bfr_size, MPI_BYTE, master, CHDB_TAG_END, MPI_COMM_WORLD);

	// free memory
	free(bfr);
}

/** 
 * @brief Execute the command on all files of file_pathes, store the result in return_values and may be the wall time in wall_times
 *        This function should be called by the slaves only
 * 
 */
void BasicScheduler::executeCommand() {

	// Create the output and temporary directories when first-time call
	// If the output directory already exists, it will be removed
	// NOTE - The creation of those directories is defferred as much as possible,
	// so that the master can execute an MPI_Abort during initialization, without 
	// letting temporaries on the disk !
	if (first_execution) {
		dir.makeOutDir(true,true);
		dir.makeTempOutDir();
		first_execution = false;
	}

	string command = prms.getExternalCommand();
	int zero = 0;
	double zerod = 0.0;
	return_values.assign (file_pathes.size(),zero);

	if (prms.isReportMode()) {
		wall_times.assign(file_pathes.size(),zerod);
	}

	for (size_t i=0; i<file_pathes.size(); ++i) {
		string cmd = command;
		string in_path = file_pathes[i];

		// skip input files with empty names
		if (in_path.size()==0) continue;

		dir.completeFilePath(in_path,cmd);
		vector_of_strings out_files = prms.getOutFiles();
		for (size_t j=0; j<out_files.size(); ++j) {
			dir.completeFilePath(in_path,out_files[j],true);
		}
		double start = MPI_Wtime();
		int sts = dir.executeExternalCommand(cmd,out_files);
		double end = MPI_Wtime();
		// If abort on Error, throw an exception if status != 0
		if (sts!=0) {
			//	if (prms.isAbrtOnErr()) {
			//	ostringstream msg;
			//	msg << "ERROR with external command\n";
			//	msg << "command    : " << cmd << '\n';
			//	msg << "exit status: " << sts << '\n';
			//	msg << "ABORTING - Try again with --on-error\n";
			//	throw(runtime_error(msg.str()));
			//} else {
			return_values[i] = sts;
			//}
		}

		// Store the time elapsed if report mode
		if ( prms.isReportMode() ) {
			wall_times[i] = end - start;
		}
	}
}

/** 
 * @brief Called by the master when error mode is on
 * 
 * @param err_file 
 *
 * @return true = error found, false = no error
 */
bool BasicScheduler::errorHandle(ofstream& err_file) {

	// If Abort On Error, just return. Abort already called if there was an error !
	//if (prms.isAbrtOnErr()) return;

	// find the first value in error and return false if none
	vector_of_int::iterator it = find_if(return_values.begin(),return_values.end(),isNotNull);
	if (it == return_values.end()) {
		return false;

	} else {

		// If not Abort On Error, loop from it and write the files in error
		if (!prms.isAbrtOnErr()) {
			for ( size_t i=it-return_values.begin(); i<return_values.size(); ++i) {
				if (return_values[i]==0) continue;
				err_file << *it << '\t' << file_pathes[i] << '\n';
			}

			// If Abort on Error, write a message to cerr before aborting !
		} else {
			cerr << "ERROR with external command - sts=";
			cerr << *it << " ";
			cerr << "input file=" << file_pathes[it-return_values.begin()] << '\n';
			cerr << "ABORTING - Try again with --on-error\n";
		}

		// Error found !
		return true;
	}
}

/** 
 * @brief Alloc a buffer big enough to send/receive return_values and file_pathes
 *        For block_size 4 and FILEPATH_MAXLENGTH 5 we may have at most:
 *          4000AAAABBBBCCCCDDDD4000xxxxx\0xxxxx\0xxxxx\0xxxxx\0
 *          4000 is the integer representation of 4 in little endian machines
 *          A,B,C,D are integers representing the status retrieved by the slaves for each file name
 *
 * @param[out] bfr The allocated buffer
 * @param[out] bfr_sze The size of the allocated buffer
 *
 */
void BasicScheduler::allocBfr(void*& bfr,size_t& bfr_size) {
	size_t vct_size = prms.getBlockSize();
	bfr_size  = sizeof(int) + vct_size*sizeof(int);
	bfr_size += sizeof(int) + vct_size*sizeof(double);
	bfr_size += sizeof(int) + vct_size*(FILEPATH_MAXLENGTH+1);
	bfr       = malloc(bfr_size);
	if (bfr==NULL) {
		throw(runtime_error("ERROR - Could not allocate memory"));
	}
}

/** 
 * @brief Write to a send buffer the vectors return_values, may be wall_times, and file_pathes
 * 
 * @pre The bfr should be already allocated with allocBfr

 * @param[in]  bfr       The buffer (should be already allocated)
 * @param[in]  bfr_size  The buffer length
 * @param[out] data_size The data stored, should be <= bfr_size
 * 
 */
void BasicScheduler::writeToSndBfr(void* bfr, size_t bfr_size, size_t& data_size) {
	checkInvariant();
	size_t int_data_size=0;
	size_t str_data_size=0;
	size_t dbl_data_size=0;

	// Mode No Report:
	// ---------------
	//
	// Fill the buffer with the data from return_values, then from file_pathes
	//      iiiiiiiiifffffffffffffffffffffffffff00000000000000000000000000000
	//      ^        ^                          ^
	//      0        int_data_size              int_data_size+str_data_size  ^bfr_size
	//
	// Mode --report:
	// --------------
	//
	// Fill the buffer with the data from return_values, from wall_times, then from file_pathes
	//     iiiiiiiiidddddddddfffffffffffffffffffffffffff00000000000000000000000000000
	//     ^        ^        ^-----v                    ^-------v                   ^----------------------v
	//     0        int_data_size  int_data_size+dbl_data_size  int_data_size+dbl_data_size+str_data_size  bfr_size
	
	vctToBfr(return_values,bfr,bfr_size,int_data_size);
	bfr      = (void*) ((char*) bfr + int_data_size);
	bfr_size = bfr_size - int_data_size;
	data_size= int_data_size;
	if (prms.isReportMode()) {
		vctToBfr(wall_times,bfr,bfr_size,dbl_data_size);
		bfr = (void*)((char*) bfr + dbl_data_size);
		bfr_size = bfr_size - dbl_data_size;
		data_size += dbl_data_size;
	}
	vctToBfr(file_pathes,bfr,bfr_size,str_data_size);
	data_size += str_data_size;
}

/** 
 * @brief Read a receive buffer and initialize the vectors return_values, may be wall_times, and file_pathes
 * 
 * @param bfr 
 */
void BasicScheduler::readFrmRecvBfr	(const void* bfr) {
	size_t data_size;
	bfrToVct(bfr,data_size,return_values);
	bfr = (void*) ((char*) bfr + data_size);
	if (prms.isReportMode()) {
		bfrToVct(bfr,data_size,wall_times);
		bfr = (void*) ((char*) bfr + data_size);
	}
	bfrToVct(bfr, data_size, file_pathes);
	checkInvariant();
}

/** 
 * @brief The invariants:
 *              1/ return_values empty, OR same size as file_pathes
 *              2/ wall_times empty, OR same size as file_pathes
 *
 */
void BasicScheduler::checkInvariant() {
	assert(return_values.empty() || return_values.size()==file_pathes.size());
	assert(wall_times.empty() || wall_times.size()==file_pathes.size());
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
