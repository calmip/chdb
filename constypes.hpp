/**
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

#ifndef CONSTYPES_CHDB_H
#define CONSTYPES_CHDB_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
using namespace std;

#include "version.hpp"

/** 
 * @brief Some predicates
 * 
 * @param s 
 * 
 * @todo should work with the same name, but we get the error "unresolved function type"
 * @return 
 */

inline bool isNotNullStr (const string& s) { return (s.size()>0); }
inline bool isNotNull(int i) { return (i!=0); }

/* 
   NOTMP => The CALMIP policy is not to use /tmp on the nodes
            Comment out the following line to regain tmpdir functionality
*/
#define NOTMP

/*
   comment out to avoid creating an output directory per slave
*/
//#define OUTDIRPERSLAVE 1

/*
  number of retries executing the external command if there is an error in system()
*/
#define NUMBER_OF_RETRIES 5

/**
   \brief To have a class non copyable, you just have to private derive from this class
*/

class NonCopyable {
public:
	NonCopyable(){};

private:
	NonCopyable(const NonCopyable&) {};
	void operator=(const NonCopyable&) {};
};

/* Some typedefs */
typedef vector<string> vector_of_strings;
typedef vector<int> vector_of_int;
typedef vector<double> vector_of_double;


#endif
