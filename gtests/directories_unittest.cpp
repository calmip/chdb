
/**
   unit tests for the class Directories, and its subclasses
*/

#include "constypes_unittest.hpp"
#include "../usingfs.hpp"
#include <fstream>
using namespace std;

TEST_F(ChdbTest,getFiles_Unsorted) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	// No sort
	Parameters prms(7,argv);
	UsingFs dir(prms);
	vector_of_strings found_files=dir.getFiles();
	vector_of_strings expected_files;
	expected_files.push_back(input_dir + '/' + "B.txt");
	expected_files.push_back(input_dir + '/' + "C/C.txt");
	expected_files.push_back(input_dir + '/' + "C/C/C.txt");
	expected_files.push_back(input_dir + '/' + "D/C.txt");
	expected_files.push_back(input_dir + '/' + "A.txt");
	EXPECT_EQ(expected_files,found_files);

	// again
	found_files=dir.getFiles();
	EXPECT_EQ(expected_files,found_files);

	FREE_ARGV(7);

}

TEST_F(ChdbTest,getFiles_Sorted) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--sort-by-size");
	
   	// Sort by size
	Parameters prms(8,argv);
	UsingFs dir(prms);
	vector_of_strings found_files=dir.getFiles();
	vector_of_strings expected_files;
	expected_files.push_back(input_dir + '/' + "C/C/C.txt");
	expected_files.push_back(input_dir + '/' + "D/C.txt");
	expected_files.push_back(input_dir + '/' + "A.txt");
	expected_files.push_back(input_dir + '/' + "B.txt");
	expected_files.push_back(input_dir + '/' + "C/C.txt");
	EXPECT_EQ(expected_files,found_files);

	// again
	found_files=dir.getFiles();
	EXPECT_EQ(expected_files,found_files);

	FREE_ARGV(8);

}

// Testing blocks of 1 file (default)
TEST_F(ChdbTest,block1) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);

	vector_of_strings block;
	vector_of_strings expected_block;

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "B.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "C/C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 4th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "D/C.txt");
	EXPECT_EQ(expected_block,block);

	// 5th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "A.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(7);

}
// Testing blocks of 2 files
TEST_F(ChdbTest,block2) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"2");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);

	vector_of_strings block;
	vector_of_strings expected_block;

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "B.txt");
	expected_block.push_back(input_dir + '/' + "C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "C/C/C.txt");
	expected_block.push_back(input_dir + '/' + "D/C.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "A.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(9);

}

// Testing nextBlock, block size == total number of files
TEST_F(ChdbTest,block3) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"5");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);

	vector_of_strings block;
	vector_of_strings expected_block;

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "B.txt");
	expected_block.push_back(input_dir + '/' + "C/C.txt");
	expected_block.push_back(input_dir + '/' + "C/C/C.txt");
	expected_block.push_back(input_dir + '/' + "D/C.txt");
	expected_block.push_back(input_dir + '/' + "A.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(9);

}

// Testing nextBlock, block size > total number of files
TEST_F(ChdbTest,block4) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"10");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);

	vector_of_strings block;
	vector_of_strings expected_block;

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back(input_dir + '/' + "B.txt");
	expected_block.push_back(input_dir + '/' + "C/C.txt");
	expected_block.push_back(input_dir + '/' + "C/C/C.txt");
	expected_block.push_back(input_dir + '/' + "D/C.txt");
	expected_block.push_back(input_dir + '/' + "A.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(9);

}

// Testing 
TEST_F(ChdbTest,completeFilePath) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);

	string f1 = "A/B/C/D.txt";

	string s1 = "outputdir/#path#";
	string s2 = "outputdir/#name#";
	string s3 = "outputdir/#basename#.out";
	string s4 = "outputdir/#dirname#/#basename#.out";
	string s5 = s1 + "##" + s1 + "##" + s2 + "##" + s2 + "##" + s3 + "##" + s3;
	dir.completeFilePath(f1,s1);
	dir.completeFilePath(f1,s2);
	dir.completeFilePath(f1,s3);
	dir.completeFilePath(f1,s4);
	dir.completeFilePath(f1,s5);

	string expected_s1 = "outputdir/A/B/C/D.txt";
	string expected_s2 = "outputdir/D.txt";
	string expected_s3 = "outputdir/D.out";
	string expected_s4 = "outputdir/A/B/C/D.out";
	string expected_s5 = expected_s1 + "##" + expected_s1 + "##" + expected_s2 + "##" + expected_s2 + "##" + expected_s3 + "##" + expected_s3;
	vector_of_strings block;
	vector_of_strings expected_block;

	EXPECT_EQ(s1,expected_s1);
	EXPECT_EQ(s2,expected_s2);
	EXPECT_EQ(s3,expected_s3);
	EXPECT_EQ(s4,expected_s4);
	EXPECT_EQ(s5,expected_s5);

	FREE_ARGV(7);

}



#ifdef AJETER		



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

#endif
