#ifndef CONSTYPES_UNITTEST_H
#define CONSTYPES_UNITTEST_H

/**
   \brief Some includes, common to ALL tests
*/

//#include <cstdlib>
#include <string>
#include <map>
//#include <memory>
using namespace std;
#include "gtest/gtest.h"
#include "../command.hpp"

#define INPUTDIR1 "inputdir1"

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

// Comment out the following to inhibit directory cleaning (but this will make fail some tests)
#define REMOVE_OUTPUT

/***** TEST FIXTURES *****/

// Test fixture base class -> used by most tests
class BdbhTest: public ::testing::Test {
protected:
	BdbhTest(const string& n=INPUTDIR1);
	virtual ~BdbhTest();
	virtual string getInputDir() const { return input_dir; };
	virtual string getDbDir() const { return input_dir + ".db"; };

protected:
	vector<naco> files;

private:
	string input_dir;
};

/** 
 * @brief To be used with TEST_P macros
 * 
 */
class TestCase1: public BdbhTest,
				 public ::testing::WithParamInterface<bool> {
public:
	TestCase1() : BdbhTest(),::testing::WithParamInterface<bool>() {};
	~TestCase1() {};
};

#endif
