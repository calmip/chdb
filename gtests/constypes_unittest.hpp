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
#include "../constypes.hpp"
#include "../system.hpp"
#include "../parameters.hpp"
#include "../directories.hpp"
#include "../usingfs.hpp"
#include "../usingbdbh.hpp"
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
string readFileFromBdbh(const string& db, const string& key);
bool existsFile(const string&);
bool existsFileFromBdbh(const string& db, const string& key);
void removeFile(const string&);

// some defines: The input directory names
#define INPUTDIR1 "inputdir1"
#define INPUTDIR2 "inputdir2"
#define INPUTDIR3 "inputdir3"

// Comment out the following to inhibit directory cleaning (but this will make fail some tests)
#define REMOVE_OUTPUT

/***** TEST FIXTURES *****/

// Test fixture base class -> used by most tests
// getInputDir() may be overloaded by subclasses, so you may have several objects for 1 input directory
// For instance = 1 REAL directory + the corresponding bdbh version of this directory !
class ChdbTest: public ::testing::Test {
protected:
	ChdbTest(const string& n): input_dir(n){};
	virtual ~ChdbTest() { removeFile(getInputDir()+".out*"); };
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
};

// ChdbTest2 = Create 10 files in the input directory, file 0.txt in error
class ChdbTest2: public ChdbTest {
public:
	ChdbTest2();

protected:
	vector<string> expected_file_pathes;
	map<string,string> expected_file_contents;
};

// ChdbTest2 = Create 10 files in the input directory, file 9.txt in error
class ChdbTest3: public ChdbTest {
public:
	ChdbTest3();

protected:
	vector<string> expected_file_pathes;
	map<string,string> expected_file_contents;
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
class SchedTestDbl : public ChdbTest1 {
public:
	SchedTestDbl();
	~SchedTestDbl() { free(bfr); };

protected:
	vector_of_double expected_values;
	
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

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
 *        Do note forget to DELETE it
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
class ChdbTestsWithParamsUsingBdbh: public ChdbTestsWithParams {
public:
	ChdbTestsWithParamsUsingBdbh(const string& t): ChdbTestsWithParams(t) {};
	virtual Directories* createDirectory(const Parameters& p) { return new UsingBdbh(p); };
	virtual string cmplInputDir(const string& n) { string s=n+".db"; return s;};
	virtual string getDescription() const {string rvl="TESTING USINGBDBH ";rvl+="TEMPORARY=";rvl+=getTmpDir();return rvl;};
	virtual string getDirectoryType() const {return (string) "UsingBdbh";};
	virtual void cvtInputDir(const string& src);
};

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

#endif
