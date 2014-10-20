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

#include <cassert>

//#include "gtest/gtest.h"

//#include "SimpleOpt.h"
#include "constypes.hpp"
#include "parameters.hpp"
#include "directories.hpp"

void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size);
void bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& files_names);

template <typename T> void vctToBfr(const vector<T>& values, void* bfr, size_t bfr_size, size_t& data_size);
template <typename T> void bfrToVct(void const* bfr, size_t& data_size, vector<T>& values);

class Scheduler: private NonCopyable {
public:
	Scheduler(const Parameters& p, Directories& d);
	int getRank() const     { return rank; };
	size_t getNbOfSlaves() const { return comm_size-1; };
	bool isMaster() const   { return rank==0;};

	static void init(int,char**);
	static void finalize();
	static void abort();

	virtual void mainLoop()=0;
	virtual void errorHandle(ofstream&)=0;
	virtual size_t getTreatedFiles() const=0;

	void startTimer();
	double getTimer();

	// for gtests
	//friend class SchedTestStr_vctToBfrStrings_Test;
	//friend class SchedTestStr_bfrToVctStrings_Test;
	//friend class SchedTestInt_vctToBfrInt_Test;
	//friend class SchedTestInt_bfrToVctInt_Test;

protected:

	const Parameters& prms;
	Directories& dir;

	int comm_size;
	int rank;

private:
	double start_time;

};


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
template <typename T> void vctToBfr(const vector<T>& values, void* bfr, size_t bfr_size, size_t& data_size) {
	// does not work for all T !
	assert(sizeof(T) >= sizeof(int));

	size_t vct_sze  = values.size();

	data_size=(vct_sze+1) * sizeof(T);
	if (data_size > bfr_size) {
		throw(runtime_error("ERROR - Buffer too small"));
	}
	T * v = (T*) bfr;
	v[0]  = (T) vct_sze;
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
template <typename T> void bfrToVct(void const* bfr, size_t& data_size, vector<T>& values) {
	// does not work for all T !
	assert(sizeof(T) >= sizeof(int));

	values.clear();
	T* v = (T*) bfr;
	int sze = (int) v[0];
	for (int i=0; i<sze; ++i) {
		values.push_back(v[i+1]);
	}
	data_size = sizeof(T)*(values.size()+1);
}

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
