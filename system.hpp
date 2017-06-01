

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include "constypes.hpp"
using namespace std;

#include <cassert>

void getHostName(string& h);
void getCurrentDirName(string& d);
void sleepMs(unsigned int);
int  callSystem(string cmd, bool err_flg=false);
void replaceTmpl(const string& tmpl, const string& value, string& text);
void parseFilePath(const string& path, string& dir, string& name, string& base, string& ext);
bool isEndingWith(const string& name, const string& ext);
bool isBeginningWith(const string& name, const string& heading);
vector_of_strings split(const string &);
bool fileExists(const string&);
void mkdir(const string&, mode_t mode=0777);
void vctToBfr(const vector_of_strings& file_pathes, void* bfr, size_t bfr_size, size_t& data_size);
void bfrToVct(void const* bfr, size_t& data_size, vector_of_strings& files_names);

template <typename T> void vctToBfr(const vector<T>& values, void* bfr, size_t bfr_size, size_t& data_size);
template <typename T> void bfrToVct(void const* bfr, size_t& data_size, vector<T>& values);

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
