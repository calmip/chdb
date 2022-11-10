/** This file is part of chdb software
 * 
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

/**
   unit tests for the class Scheduler, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../src/system.hpp"
#include "../src/basicscheduler.hpp"
// the following to know if OUTDIRPERSLAVE is defined
#include "../src/usingfs.hpp"
#include <fstream>
#include <algorithm>
using namespace std;

using ::testing::TestWithParam;
using ::testing::Values;

// GLOBAL TEST ENVIRONMENT SETUP
// For global initialization of the MPI library
class Environment : public ::testing::Environment {
 public:
  ~Environment() override {}

  // Override this to define how to set up the environment.
  void SetUp() override {
      Scheduler::init(0,nullptr);
  }

  // Override this to define how to tear down the environment.
  void TearDown() override {
      Scheduler::finalize();

  }
};

testing::Environment* const env =
    testing::AddGlobalTestEnvironment(new Environment);
// --------------------------------

// Test basicscheduler::readFrmRecvBfr and basicscheduler::writeToSndBfr
TEST_F(ChdbTest1,readwriteToSndBfr) {

    // Init prms
    char* argv[11];
    INIT_ARGV(0,"scheduler_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,getInputDir().c_str());
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");

    Parameters prms(7,argv);
    UsingFs dir(prms);

    // Load (empty) values and file_pathes from expected_bfr
    BasicScheduler sched(prms,dir,true);

    vector_of_strings s1, s2, rec_s1, rec_s2;
    vector_of_int i1, rec_i1;
    vector_of_double d1, rec_d1;

    size_t bfr_size = 100;
    unique_ptr<char[]> bfr_mngr = make_unique<char[]>(bfr_size);
    void* bfr = static_cast<void*>(bfr_mngr.get());
    size_t data_size = 0;

    // We serialize i1, d1, s1, s2 using writeToSndBfr
    // Then we call readFrmRecvBfr to initialize rec_i1, rec_d1, rec_s1, rec_s2
    // At last we compare i1/rec_i1, d1/rec_d1, s1/rec_1, s2/rec_s2
    sched.writeToSndBfr(bfr, bfr_size, data_size, i1, d1, s1, s2);
    sched.readFrmRecvBfr(bfr, rec_i1, rec_d1, rec_s1, rec_s2);
    EXPECT_EQ(i1, rec_i1);
    EXPECT_EQ(d1, rec_d1);
    EXPECT_EQ(s1, rec_s1);
    EXPECT_EQ(s2, rec_s2);

    // See checkInvariant: size of s1, s2, i1 must be the same
    s1 = {"a/b/c1.txt","x/y/z1.txt",""};
    s2 = {"toto","titi",""};
    i1 = { 0,0,1 };
    d1 = { 23, 56, 24};
    
    sched.writeToSndBfr(bfr, bfr_size, data_size, i1, d1, s1, s2);
    sched.readFrmRecvBfr(bfr, rec_i1, rec_d1, rec_s1, rec_s2);
    EXPECT_EQ(i1, rec_i1);
    EXPECT_EQ(d1, rec_d1);
    EXPECT_EQ(s1, rec_s1);
    EXPECT_EQ(s2, rec_s2);

    FREE_ARGV(7);
};

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?

