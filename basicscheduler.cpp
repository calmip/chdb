/**
 * @file   basicscheduler.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Tue Sep 30 09:57:49 2014
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
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>

//#include "command.h"
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

void BasicScheduler::init() {
	MPI_Init(NULL,NULL);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
	is_master = (rank==0);
}

void BasicScheduler::finalize() {
	MPI_Finalize();
}

void BasicScheduler::mainLoop() {
	if (is_master) {
		mainLoopMaster();
	} else {
		mainLoopSlave();
	}
}

void BasicScheduler::mainLoopMaster() {
	vector_of_strings blk;
	MPI_Status sts;

	// loop over the file blocks
	// Listen to the slaves, the message is Status + File names
	size_t bfr_size=0;
	void* bfr=NULL;
	allocBfr(bfr,bfr_size);
	
	while(true) {
		blk = dir.nextBlock();

		if (blk.empty()) break;

		MPI_Recv((char*)bfr,(int)bfr_size, MPI_CHAR, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
		int source = sts.MPI_SOURCE;
		size_t msg_len;
		MPI_Get_count(&sts, MPI_CHARACTER, (int*) &msg_len);
		errorHandle(bfr,msg_len);

        // The recv bfr (bfr,msg_len) can be recycled now
		// copy the block of file names to the send bfr
		//vctToBfr(blk,bfr,bfr_size,msg_len);
		writeToSndBfr(bfr, bfr_size, msg_len);

		// Send the block to the slave
		int dest = source;
		MPI_Send(bfr,msg_len,MPI_CHARACTER,dest,CHDB_TAG_GO,MPI_COMM_WORLD);
	}

	// loop over the slaves: when each slave is ready, send it a msg END
	int working_slaves = comm_size - 1; // The master does not work
	while(working_slaves>0) {
		MPI_Recv(bfr, bfr_size, MPI_CHARACTER, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
		int source = sts.MPI_SOURCE;
		int msg_len;
		MPI_Get_count(&sts, MPI_CHARACTER, &msg_len);
		errorHandle(bfr,msg_len);

		// Send an empty message tagged END to the slave
		int dest = source;
		MPI_Send(bfr, 0, MPI_CHARACTER, dest, CHDB_TAG_END, MPI_COMM_WORLD);
		working_slaves--;
	}
}

void BasicScheduler::mainLoopSlave() {
	vector_of_strings blk;
	MPI_Status sts;

	size_t bfr_size=0;
	void* bfr=NULL;
	allocBfr(bfr,bfr_size);

	// all msgs are sent to the master or received from him
	int dest = 0;
	int source = 0;
	while(true) {

		//writeBfr(bfr,xxx);
		MPI_Sendrecv_replace(bfr,bfr_size,MPI_CHARACTER,
							 dest,CHDB_TAG_READY,
							 source,MPI_ANY_TAG,
							 MPI_COMM_WORLD,&sts);

	}
}

/** 
 * @brief Alloc a buffer big enough to send/receive return_values and file_names
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
	bfr_size += sizeof(int) + vct_size*(FILEPATH_MAXLENGTH+1);
	bfr       = malloc(bfr_size);
	if (bfr==NULL) {
		throw(runtime_error("ERROR - Could not allocate memory"));
	}
}

/** 
 * @brief The invariant is: return_values empty, OR same size as file_names
 *
 */
void BasicScheduler::checkInvariant() {
	assert(return_values.empty() || return_values.size()==file_names.size());
}

/** 
 * @brief Write to a send buffer the vectors return_values and file_names
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

	// Fill the buffer with the data from return_values, then from file_names
	//      iiiiiiiiifffffffffffffffffffffffffff00000000000000000000000000000
	//      ^        ^                          ^
	//      0        int_data_size              int_data_size+str_data_size  ^bfr_size
	//                               
	
	vctToBfr(return_values,bfr,bfr_size,int_data_size);
	bfr = (void*) ((char*) bfr + int_data_size);
	bfr_size = bfr_size - int_data_size;
	vctToBfr(file_names,bfr,bfr_size,str_data_size);
	data_size = int_data_size + str_data_size;
}

void BasicScheduler::readFrmRecvBfr	(const void* bfr) {
	size_t data_size;
	bfrToVct(bfr,data_size,return_values);
	bfr = (void*) ((char*) bfr + data_size);
	bfrToVct(bfr, data_size, file_names);
	checkInvariant();
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

