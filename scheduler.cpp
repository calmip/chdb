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
 * @brief throw an error if MPI not yet initialized or if no slave 
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

/** 
 * @brief Write in a Buffer for sending a block of file names
 *        We store the number of strings (as an integer), then the strings
 *        Storing three files, little endian: 3000xxx\0xxx\0xxx\0
 * 
 * @param[in] files_names A vector of file names
 * @param[in] bfr The buffer (should be already allocated)
 * @param[in] bfr_size The buffer size
 * @param[out] data_size The length of data stored
 * @exception A runtime_exception is thrown if the buffer is too small
 * 
 */
void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size ) {
	data_size=sizeof(int);
	for (size_t i=0; i<file_pathes.size(); ++i) {
		data_size += file_pathes[i].length()+1;
	}

	if (data_size > bfr_size) {
		throw(runtime_error("ERROR - Buffer too small"));
	}

	size_t vct_sze = file_pathes.size();
	memcpy(bfr,(void*)&vct_sze,sizeof(int));
	size_t offset=sizeof(int);
	for (size_t i=0;i<vct_sze; ++i) {
		strcpy((char*)bfr+offset,file_pathes[i].c_str());
		offset += file_pathes[i].length()+1;
	}
}

/** 
 * @brief Create a vector of file names from a receive buffer
 * 
 * @param[in]  bfr         The buffer
 * @param[out] data_size   The size of data read in bfr
 * @param[out] files_names 
 */
void bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& file_pathes) {
	int sze=0;
	memcpy((void*)&sze,bfr,sizeof(int));
	size_t l_tmp = sizeof(int);
	file_pathes.clear();
	for (int i=0; i<sze; ++i) {
		char const* b_tmp = (char const *)bfr+l_tmp;
		file_pathes.push_back(b_tmp);
		l_tmp += strlen(b_tmp)+1;
	}
	data_size = l_tmp;
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

