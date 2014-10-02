

// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.
//
// Writing a unit test using Google C++ testing framework is easy as 1-2-3:


// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

//#include <limits.h>
#include <cstdlib>
#include <cstring>
#include "../constypes.hpp"
#include "../parameters.hpp"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>

// This macro is useful to initialize argv with read-write area, this is required by simpleOpt
// #define INIT_ARGV(I,VAL) { char a[]=VAL;argv[I] = a; };
#define INIT_ARGV(I,VAL) argv[I]=(char*)malloc(strlen(VAL)+1);strcpy(argv[I],VAL);
#define FREE_ARGV(ARGC) for(int i=0;i<ARGC;i++){free(argv[i]); argv[i]=NULL;}

// Tests constructors
TEST(ParametersTest, CtorExceptions1) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--in-type");
	INIT_ARGV(2,"txt");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--command-line");
	INIT_ARGV(6,"coucou");

	// "no command line" exception
	ASSERT_THROW(new Parameters(5,argv),runtime_error);
	ASSERT_THROW(new Parameters(6,argv),runtime_error);

	// If you comment-out the following, it CANNOT BE compiled !
	// Parameters prms1 = Parameters(7,argv);
	// Parameters prms2 = prms1;
}
TEST(ParametersTest, CtorExceptions2) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--in-type");
	INIT_ARGV(2,"txt");
	INIT_ARGV(3,"--command-line");
	INIT_ARGV(4,"coucou");
	INIT_ARGV(5,"--in-dir");
	INIT_ARGV(6,"inputdir");

	// "no input directory" exception
	ASSERT_THROW(new Parameters(5,argv),runtime_error);

	// "input directory does not exist" exception
	system("rm -rf inputdir");
	ASSERT_THROW(new Parameters(7,argv),runtime_error);

	// creating the directory
	system("mkdir inputdir");
	Parameters prms(7,argv);
	EXPECT_EQ((string)"inputdir",prms.getInDir());

	FREE_ARGV(7);
}
TEST(ParametersTest, CtorExceptions3) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--in-type");
	INIT_ARGV(2,"txt");
	INIT_ARGV(3,"--command-line");
	INIT_ARGV(4,"coucou");
	INIT_ARGV(5,"--in-dir");
	INIT_ARGV(6,"inputdir");

    // automatically computed name
	string outputdir = "inputdir";
	outputdir += ".out";

	// "output directory exists" exception: creating the output directory
	string cmd = "mkdir ";
	cmd += outputdir;
	system(cmd.c_str());
	ASSERT_THROW(new Parameters(7,argv),runtime_error);

	cmd = "rm -r ";
	cmd += outputdir;
	system(cmd.c_str());
	Parameters prms(7,argv);
	EXPECT_EQ(outputdir,prms.getOutDir());

	FREE_ARGV(7);
}
TEST(ParametersTest, CtorExceptions4) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");

	// "no input type" exception
	ASSERT_THROW(new Parameters(5,argv),runtime_error);
	Parameters prms(7,argv);

	// empty vector_of_files
	vector_of_strings out_files;

	// checking the accessors
	EXPECT_EQ("coucou",prms.getExternalCommand());
	EXPECT_EQ(1,prms.getBlockSize());
	EXPECT_EQ(false,prms.isSizeSort());
	EXPECT_EQ(false,prms.isVerbose());
	EXPECT_EQ(true,prms.isAbrtOnErr());
	EXPECT_EQ(out_files,prms.getOutFiles());	

	FREE_ARGV(7);
}
TEST(ParametersTest, CtorBlockSize) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"10");
	Parameters prms(9,argv);

	// checking the block size accessor
	EXPECT_EQ(10,prms.getBlockSize());

	// should throw an exception if block-size <= 0
	FREE_ARGV(9);
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"-10");
	ASSERT_THROW(new Parameters(9,argv),runtime_error);

	// should throw an exception if block-size == 0
	FREE_ARGV(9);
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"0");
	ASSERT_THROW(new Parameters(9,argv),runtime_error);

	FREE_ARGV(9);
}

TEST(ParametersTest, CtorOnError) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--on-error");
	INIT_ARGV(8,"errors.txt");

	// If the switch --on-error IS NOT specified, isAbrtOnErr() returns true
	Parameters prms1(7,argv);
	EXPECT_EQ(true,prms1.isAbrtOnErr());

	// If the switch --on-error IS specified, isAbrtOnErr() returns false
	// "no input type" exception
	Parameters prms2(9,argv);
	EXPECT_EQ(false,prms2.isAbrtOnErr());

	FREE_ARGV(9);
}
TEST(ParametersTest, CtorFlags) {
	char* argv[10];
	INIT_ARGV(0,"parameters_unittest");
	INIT_ARGV(1,"--in-type");
	INIT_ARGV(2,"txt");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--command-line");
	INIT_ARGV(6,"coucou");
	INIT_ARGV(7,"--sort-by-size");
	INIT_ARGV(8,"--verbose");
	
	Parameters prms(9,argv);
	ASSERT_EQ(true,prms.isSizeSort());
	ASSERT_EQ(true,prms.isVerbose());
}

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