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
void Scheduler::vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size ) {
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
void Scheduler::bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& file_pathes) {
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

/** 
 * @brief Write in a Buffer for sending a block of integers
 *        We store the number of integers, THEN the integers
 *        Storing three int, little endian: 3000xxx\0xxx\0xxx\0 
 * 
 * @param[in] values 
 * @param[in] bfr 
 * @param[in] bfr_size 
 * @param[out] data_size The length of data stored
 * @exception A runtime_exception is thrown if the buffer is too small
 *
 */
void Scheduler::vctToBfr(const vector_of_int& values, void* bfr, size_t bfr_size, size_t& data_size ) {
	size_t vct_sze  = values.size();
	data_size= (vct_sze+1) * sizeof(int);
	if (data_size > bfr_size) {
		throw(runtime_error("ERROR - Buffer too small"));
	}

	int* v = (int*) bfr;
	v[0] = (int) vct_sze;
	for (size_t i=0; i<vct_sze; ++i) {
		v[i+1] = values[i];
	}
}

/** 
 * @brief Create a vector of int from a receive buffer
 * 
 * @param[in]  bfr         The buffer
 * @param[out] data_size   The size of data read in bfr
 * @param[out] values
 *
 */
void Scheduler::bfrToVct(void const* bfr, size_t& data_size, vector_of_int& values) {
	int *v = (int*) bfr;
	int sze = v[0];
	for (int i=0; i<sze; ++i) {
		values.push_back(v[i+1]);
	}
	data_size = sizeof(int)*(values.size()+1);
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

