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

#ifndef CONSTYPES_UNITTEST_H
#define CONSTYPES_UNITTEST_H

/**
   \brief Some includes, common to ALL tests
*/

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <memory>
#include "../src/constypes.hpp"
#include "../src/system.hpp"
#include "../src/parameters.hpp"
#include "../src/directories.hpp"
#include "../src/usingfs.hpp"
// REMOVED #include "../src/usingbdbh.hpp"
#include "gtest/gtest.h"

// This macro is useful to initialize argv with read-write area, this is required by simpleOpt
// #define INIT_ARGV(I,VAL) { char a[]=VAL;argv[I] = a; };
#define INIT_ARGV(I,VAL) argv[I]=(char*)malloc(strlen(VAL)+1);strcpy(argv[I],VAL);
#define FREE_ARGV(ARGC) for(int i=0;i<ARGC;i++){free(argv[i]); argv[i]=NULL;}

// the following is used by some test programs
// file NAme and COntent
struct naco {
    naco(const string& n, const string& c) : name(n),content(c) {};
    string name;
    string content;
};

// convenient functions
void createFile(const string& d, const naco& n);
string readFile(const string&);
// REMOVED string readFileFromBdbh(const string& db, const string& key);
bool existsFile(const string&);
// REMOVED bool existsFileFromBdbh(const string& db, const string& key);
void removeFile(const string&);

// some defines: The input directory names
#define INPUTDIR1 "inputdir1"
#define INPUTDIR2 "inputdir2"
#define INPUTDIR3 "inputdir3"
#define INPUTDIR4 "inputdir4"

// Comment out the following to inhibit directory cleaning (but this will make fail some tests)
#define REMOVE_OUTPUT

/***** TEST FIXTURES *****/

// Testing the functions defined in system.hpp
// Test fixture base class -> used by most tests
// getInputDir() may be overloaded by subclasses, so you may have several objects for 1 input directory
// For instance = 1 REAL directory + the corresponding bdbh version of this directory !
class ChdbTest: public ::testing::Test {
protected:
    ChdbTest(const string& n): input_dir(n){/* REMOVED bdbh::Initialize(); */ };
    virtual ~ChdbTest() { removeFile(getInputDir()+".out*");/*bdbh::Terminate();*/ };
    virtual string getInputDir() const { return input_dir; };

private:
    string input_dir;
};

// ChdbTest1 = Create 5 files in 3 subdirectories of input_dir
class ChdbTest1 : public ChdbTest {
public:
    ChdbTest1();

protected:
    vector<string> expected_file_pathes;
    map<string,string> expected_file_contents;
    map<string,string> expected_file_contents_with_rank;
};

// ChdbTest2 = Create 10 files in the input directory, file 0.txt in error
class ChdbTest2: public ChdbTest {
public:
    ChdbTest2();

protected:
    vector<string> expected_file_pathes;
    map<string,string> expected_file_contents;
};

// ChdbTest3 = Create 10 files in the input directory, file 9.txt in error
class ChdbTest3: public ChdbTest {
public:
    ChdbTest3();

protected:
    vector<string> expected_file_pathes;
    map<string,string> expected_file_contents;
};

// ChdbTest4 = Create 5 directories .dir in the input directory, nothing in the directories
class ChdbTest4: public ChdbTest {
public:
    ChdbTest4();
    
protected:
    vector<string> expected_input_files;
    vector<string> expected_file_pathes;
//    map<string,string> expected_file_contents;
};

/**
 * \brief Subclasses will be used as parameters
 */
class ChdbTestsWithParams {
public:
    ChdbTestsWithParams(const string& t): tmp_dir(t) {};
    string getTmpDir() const { return tmp_dir; };
    virtual Directories* createDirectory(const Parameters&)=0;
    virtual string getDescription() const = 0;
    virtual string getDirectoryType() const = 0;

    virtual void cvtInputDir(const string&) = 0;
    virtual string cmplInputDir(const string&) = 0;

private:
    string tmp_dir;
};

/**
 * @brief This object encapsulates the Directory creation but it NOT reponsible for the directory created !
 *        Do not forget to DELETE it
 */
class ChdbTestsWithParamsUsingFs: public ChdbTestsWithParams {
public:
    ChdbTestsWithParamsUsingFs(const string& t): ChdbTestsWithParams(t) {};
    virtual Directories* createDirectory(const Parameters& p) { return new UsingFs(p); };
    virtual string cmplInputDir(const string& n) {return n;};
    virtual void cvtInputDir(const string&) {};
    virtual string getDescription() const {string rvl="TESTING USINGFS ";rvl+="TEMPORARY=";rvl+=getTmpDir();return rvl;};
    virtual string getDirectoryType() const {return (string) "UsingFs";};
};
// REMOVED class ChdbTestsWithParamsUsingBdbh: public ChdbTestsWithParams {
// REMOVED public:
// REMOVED     ChdbTestsWithParamsUsingBdbh(const string& t): ChdbTestsWithParams(t) {};
// REMOVED     virtual Directories* createDirectory(const Parameters& p) { return new UsingBdbh(p); };
// REMOVED     virtual string cmplInputDir(const string& n) { string s=n+".db"; return s;};
// REMOVED     virtual string getDescription() const {string rvl="TESTING USINGBDBH ";rvl+="TEMPORARY=";rvl+=getTmpDir();return rvl;};
// REMOVED     virtual string getDirectoryType() const {return (string) "UsingBdbh";};
// REMOVED     virtual void cvtInputDir(const string& src);
// REMOVED };

/** 
 * @brief To be used with TEST_P macros
 * 
 */
 
class TestCase1: public ChdbTest1,
                 public ::testing::WithParamInterface<ChdbTestsWithParams*> {
public:
    TestCase1() : ChdbTest1(),::testing::WithParamInterface<ChdbTestsWithParams*>() {
        GetParam()->cvtInputDir(ChdbTest1::getInputDir());
    };
    ~TestCase1() { removeFile(getInputDir()+".out*"); };
    virtual string getInputDir() { return GetParam()->cmplInputDir(ChdbTest1::getInputDir()); };
};
class TestCase2: public ChdbTest2,
                 public ::testing::WithParamInterface<ChdbTestsWithParams*> {
public:
    TestCase2() : ChdbTest2(),::testing::WithParamInterface<ChdbTestsWithParams*>() {
        GetParam()->cvtInputDir(ChdbTest2::getInputDir());
    };
    ~TestCase2() { removeFile(getInputDir()+".out*"); };
    virtual string getInputDir() { return GetParam()->cmplInputDir(ChdbTest2::getInputDir()); };
};
class TestCase3: public ChdbTest3,
                 public ::testing::WithParamInterface<ChdbTestsWithParams*> {
public:
    TestCase3() : ChdbTest3(),::testing::WithParamInterface<ChdbTestsWithParams*>() {
        GetParam()->cvtInputDir(ChdbTest3::getInputDir());
    };
    ~TestCase3() { removeFile(getInputDir()+".out*"); };
    virtual string getInputDir() { return GetParam()->cmplInputDir(ChdbTest3::getInputDir()); };
};
class TestCase4: public ChdbTest4,
                 public ::testing::WithParamInterface<ChdbTestsWithParams*> {
public:
    TestCase4() : ChdbTest4(),::testing::WithParamInterface<ChdbTestsWithParams*>() {
        GetParam()->cvtInputDir(ChdbTest4::getInputDir());
    };
    ~TestCase4() { removeFile(getInputDir()+".out*"); };
    virtual string getInputDir() { return GetParam()->cmplInputDir(ChdbTest4::getInputDir()); };
};

// Adding the mpi buffer to ChdbTest1 (names only)
class SchedTestStr : public ChdbTest1 {
public:
    SchedTestStr();
    ~SchedTestStr() { free(bfr); };

protected:
    string expected_bfr;
    void*  bfr;
    size_t bfr_len;
};

// Adding the mpi buffer to ChdbTest1 (return values only)
class SchedTestInt : public ChdbTest1 {
public:
    SchedTestInt();
    ~SchedTestInt() { free(bfr); };

protected:
    vector_of_int expected_values;
    
    string expected_bfr;
    void*  bfr;
    size_t bfr_len;
};

// Adding the mpi buffer to ChdbTest1 (doubles for time only)
// REMOVED class SchedTestDbl : public ChdbTest1 {
// REMOVED public:
// REMOVED     SchedTestDbl();
// REMOVED     ~SchedTestDbl() { free(bfr); };
// REMOVED 
// REMOVED // REMOVED protected:
// REMOVED     vector_of_double expected_values;
// REMOVED     
// REMOVED     string expected_bfr;
// REMOVED     void*  bfr;
// REMOVED     size_t bfr_len;
// REMOVED };

// Adding the mpi buffer to ChdbTest1 (names + return values)
class SchedTestStrInt : public ChdbTest1 {
public:
    SchedTestStrInt();
    ~SchedTestStrInt() { free(bfr); };

protected:
    vector_of_strings expected_file_pathes;
    vector_of_int expected_values_1;
    string expected_bfr;
    string expected_bfr_1;
    void*  bfr;
    size_t bfr_len;
};

#endif
