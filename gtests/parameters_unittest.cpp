

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
#include "../src/constypes.hpp"
#include "../src/parameters.hpp"
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

class ParametersTest: public ::testing::Test {
public:
    ~ParametersTest() { system("rm -rf inputdir"); };
};
class ParametersTest1: public ::testing::Test {
public:
    ParametersTest1()  { system("mkdir inputdir"); };
    ~ParametersTest1() { system("rm -rf inputdir"); };
};

// Tests constructors
TEST_F(ParametersTest, CtorExceptions1) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--out-files");
    INIT_ARGV(6,"%out-dir%/%path%");
    INIT_ARGV(7,"--command-line");
    INIT_ARGV(8,"coucou");

    // "no command line" exception
    ASSERT_THROW(new Parameters(7,argv),runtime_error);
    ASSERT_THROW(new Parameters(8,argv),runtime_error);
    system("mkdir inputdir");
    ASSERT_NO_THROW(new Parameters(9,argv));
    system("rmdir inputdir");

    // If you comment-out the following, it CANNOT BE compiled !
    // Parameters prms1 = Parameters(7,argv);
    // Parameters prms2 = prms1;

    FREE_ARGV(9);
}
TEST_F(ParametersTest, CtorExceptions2) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--out-files");
    INIT_ARGV(6,"%out-dir%/%path%");
    INIT_ARGV(7,"--in-dir");
    INIT_ARGV(8,"inputdir");

    // "no input directory" exception
    ASSERT_THROW(new Parameters(7,argv),runtime_error);

    // "input directory does not exist" exception
    system("rm -rf inputdir");
    ASSERT_THROW(new Parameters(9,argv),runtime_error);

    // creating the directory
    system("mkdir inputdir");
    Parameters prms(9,argv);
    EXPECT_EQ((string)"inputdir",prms.getInDir());

    FREE_ARGV(9);
}
TEST_F(ParametersTest, CtorExceptions3) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--in-dir");
    INIT_ARGV(6,"inputdir");
    INIT_ARGV(7,"--out-files");
    INIT_ARGV(8,"%out-dir%/%path%");

    // "no out-files" exception
    ASSERT_THROW(new Parameters(7,argv),runtime_error);
    system("mkdir inputdir");
    ASSERT_NO_THROW(new Parameters(9,argv));
    system("rmdir inputdir");

    FREE_ARGV(9);
}
TEST_F(ParametersTest1, Ctoroutputdir) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--in-dir");
    INIT_ARGV(6,"inputdir");
    INIT_ARGV(7,"--out-files");
    INIT_ARGV(8,"%out-dir%/%path%");

    // automatically computed name
    string outputdir = "inputdir";
    outputdir += ".out";

    // "output directory exists" exception: creating the output directory
    string cmd = "mkdir ";
    cmd += outputdir;
    system(cmd.c_str());
    EXPECT_NO_THROW(new Parameters(9,argv));

    cmd = "rm -r ";
    cmd += outputdir;
    system(cmd.c_str());
    Parameters prms(9,argv);
    EXPECT_EQ(outputdir,prms.getOutDir());

    FREE_ARGV(9);
}
TEST_F(ParametersTest1, CtorExceptions4) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--out-files");
    INIT_ARGV(6,"%out-dir%/%path%");
    INIT_ARGV(7,"--in-type");
    INIT_ARGV(8,"txt");

    // "no input type" exception
    ASSERT_THROW(new Parameters(7,argv),runtime_error);

    Parameters prms(9,argv);
    
    // "getIterationStart etc. 
    ASSERT_EQ(false,prms.isTypeIter());
    ASSERT_THROW(prms.getIterationStart(),logic_error);
    ASSERT_THROW(prms.getIterationEnd(),logic_error);
    ASSERT_THROW(prms.getIterationStep(),logic_error);

    // empty vector_of_files
    vector_of_strings out_files;

    // checking the accessors
    EXPECT_EQ("coucou",prms.getExternalCommand());
    EXPECT_EQ(1,prms.getBlockSize());
    EXPECT_EQ(false,prms.isSizeSort());
    EXPECT_EQ(false,prms.isVerbose());
    EXPECT_EQ(true,prms.isAbrtOnErr());
    out_files.push_back("%out-dir%/%path%");
    EXPECT_EQ(out_files,prms.getOutFiles());    

    FREE_ARGV(9);
}
TEST_F(ParametersTest1, CtorBlockSize) {
    char* argv[11];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"10");
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");

    Parameters prms(11,argv);

    // checking the block size accessor
    EXPECT_EQ(10,prms.getBlockSize());

    // should throw an exception if block-size <= 0
    FREE_ARGV(11);
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"-10");
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    ASSERT_THROW(new Parameters(11,argv),runtime_error);

    // should throw an exception if block-size == 0
    FREE_ARGV(11);
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--block-size");
    INIT_ARGV(8,"0");
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    ASSERT_THROW(new Parameters(11,argv),runtime_error);

    FREE_ARGV(11);
}

TEST_F(ParametersTest1, CtorOnError) {
    char* argv[11];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--command-line");
    INIT_ARGV(2,"coucou");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--in-type");
    INIT_ARGV(6,"txt");
    INIT_ARGV(7,"--out-files");
    INIT_ARGV(8,"%out-dir%/%path%");
    INIT_ARGV(9,"--on-error");
    INIT_ARGV(10,"errors.txt");

    // If the switch --on-error IS NOT specified, isAbrtOnErr() returns true
    Parameters prms1(9,argv);
    EXPECT_EQ(true,prms1.isAbrtOnErr());

    // If the switch --on-error IS specified, isAbrtOnErr() returns false
    // "no input type" exception
    Parameters prms2(11,argv);
    EXPECT_EQ(false,prms2.isAbrtOnErr());

    EXPECT_EQ("errors.txt",prms2.getErrFile());

    FREE_ARGV(11);
}
TEST_F(ParametersTest1, CtorFlags) {
    char* argv[11];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--command-line");
    INIT_ARGV(6,"coucou");
    INIT_ARGV(7,"--sort-by-size");
    INIT_ARGV(8,"--verbose");
    INIT_ARGV(9,"--out-files");
    INIT_ARGV(10,"%out-dir%/%path%");
    
    Parameters prms(11,argv);
    ASSERT_EQ(true,prms.isSizeSort());
    ASSERT_EQ(true,prms.isVerbose());

    FREE_ARGV(11);
}

TEST_F(ParametersTest1, CtorOutFiles) {
    char* argv[10];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"txt");
    INIT_ARGV(3,"--in-dir");
    INIT_ARGV(4,"inputdir");
    INIT_ARGV(5,"--command-line");
    INIT_ARGV(6,"coucou");
    INIT_ARGV(7,"--out-files");
    INIT_ARGV(8,"out1.txt,out2.txt");
    
    Parameters prms(9,argv);

    vector_of_strings out_files;
    out_files.push_back("out1.txt");
    out_files.push_back("out2.txt");
    ASSERT_EQ(out_files,prms.getOutFiles());

    FREE_ARGV(9);
}

TEST_F(ParametersTest1, CtorIterations) {
    char* argv[7];
    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"1 10");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--out-dir");
    INIT_ARGV(6,"iter.out");

    {
        Parameters prms(7,argv);
    
        ASSERT_EQ(true,prms.isTypeIter());    
        ASSERT_EQ(1, prms.getIterationStart());
        ASSERT_EQ(1, prms.getIterationStep());
        ASSERT_EQ(10, prms.getIterationEnd());
        ASSERT_EQ("",prms.getInDir());
    }

    FREE_ARGV(7);

    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"1 10 2");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--out-dir");
    INIT_ARGV(6,"iter.out");

    {
        Parameters prms(7,argv);
    
        ASSERT_EQ(true,prms.isTypeIter());    
        ASSERT_EQ(1, prms.getIterationStart());
        ASSERT_EQ(2, prms.getIterationStep());
        ASSERT_EQ(10, prms.getIterationEnd());
    }

    FREE_ARGV(7);

    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"a b c");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--out-dir");
    INIT_ARGV(6,"iter.out");

    // Exception because a b c
    ASSERT_THROW(new Parameters(7,argv),runtime_error);

    FREE_ARGV(7);

    INIT_ARGV(0,"parameters_unittest");
    INIT_ARGV(1,"--in-type");
    INIT_ARGV(2,"1 ");
    INIT_ARGV(3,"--command-line");
    INIT_ARGV(4,"coucou");
    INIT_ARGV(5,"--out-dir");
    INIT_ARGV(6,"iter.out");

    // Exception because no end specification
    ASSERT_THROW(new Parameters(7,argv),runtime_error);

    FREE_ARGV(7);
    
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
