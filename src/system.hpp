/**
 * @file   system.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * 
 * @brief  system related functions
 *
 * This file is part of chdb software
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2022    C A L M I P
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
 * 
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <stdexcept>
#include "constypes.hpp"
using namespace std;

#include <cassert>

/*************
 * @brief This exception is thrown by callsystem if the child received a signal !
 **************************/
struct SigChildExc: public runtime_error {
    SigChildExc(int s): runtime_error(""),signal_received(s){};
    int signal_received;
};

void getHostName(string& h);
void getCurrentDirName(string& d);
void sleepMs(unsigned int);
int  callSystem(string cmd, bool err_flg=false);
void replaceTmpl(const string& tmpl, const string& value, string& text);
void parseFilePath(const string& path, string& dir, string& name, string& base, string& ext);
bool isEndingWith(const string&, const string&);
bool isBeginningWith(const string&, const string&);
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

    // data_size unchanged if there is an exception
    size_t local_data_size=(vct_sze+1) * sizeof(T);
    if (local_data_size > bfr_size) {
        throw(runtime_error("ERROR - Buffer too small"));
    }
    data_size = local_data_size;
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
