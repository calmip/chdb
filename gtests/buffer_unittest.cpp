
/**
   unit tests for the vctToBfr/bfrToVct template functions
*/


#include "constypes_unittest.hpp"
#include "../system.hpp"
#include <fstream>
using namespace std;
#include "gtest/gtest.h"


TEST_F(SchedTestStr,vctToBfrStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);
	vector_of_strings files=dir.getFiles();

	size_t data_len;
	vctToBfr(files,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(9);
};

TEST_F(SchedTestStr,bfrToVctStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	
	char const* bfr = expected_bfr.c_str();

	Parameters prms(9,argv);

	vector_of_strings file_pathes;
	size_t  data_size;
	bfrToVct((const void*)bfr,data_size,file_pathes);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_file_pathes,file_pathes);

	// 2nd time
	bfrToVct((const void*)bfr,data_size,file_pathes);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_file_pathes,file_pathes);

	FREE_ARGV(9);
};

TEST_F(SchedTestInt,vctToBfrInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);
	vector_of_strings files=dir.getFiles();

	vector_of_int values;
	values.push_back(3);
	values.push_back(5);
	values.push_back(7);

	size_t data_len;
	vctToBfr(values,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(9);
};

TEST_F(SchedTestInt,bfrToVctInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");

	char const* bfr = expected_bfr.c_str();

	Parameters prms(9,argv);

	vector_of_int values;
	size_t data_size;
	bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);

	// 2nd time
	bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);

	FREE_ARGV(9);
};

TEST_F(SchedTestDbl,bfrToVctDbl) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");

	char const* bfr = expected_bfr.c_str();

	Parameters prms(9,argv);

	vector_of_double values;
	size_t data_size;
	bfrToVct(bfr,data_size,values);
	EXPECT_EQ(expected_bfr.length(),data_size);
	EXPECT_EQ(expected_values,values);

	// 2nd time
	bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);

	FREE_ARGV(9);
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

