
/**
   unit tests for the class Directories, and its subclasses
*/

#include "constypes_unittest.hpp"
#include "../usingfs.hpp"
#include <fstream>
using namespace std;


TEST_F(ChdbTest,usingFsmkdir) {
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

	EXPECT_NO_THROW(dir.makeOutputDir());
	EXPECT_THROW(dir.makeOutputDir(),runtime_error);

	string cmd="rm -r " + prms.getOutDir();
	system(cmd.c_str());

	FREE_ARGV(7);
}

TEST_F(ChdbTest,usingFsfindOrCreateDir) {
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

	// create the directory a/b/c/d/e, then the file f.txt
	EXPECT_NO_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt"));
	{ ofstream f("a/b/c/d/e/f.txt"); }

	// nothing to do (the directories are already created)
	EXPECT_NO_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt"));

	// exception because f.txt exists and is not a directory
	// Two different pathes in the function
	EXPECT_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt/g"),runtime_error);
	EXPECT_THROW(dir.findOrCreateDir("a/b/c/d/e/f.txt/g/h"),runtime_error);

	//exception because authorization problems
	EXPECT_THROW(dir.findOrCreateDir("/etc/something/f.txt"),runtime_error);

	string cmd="rm -r a";
	system(cmd.c_str());

	FREE_ARGV(7);
}

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
	
	Parameters prms(7,argv);
	UsingFs dir(prms);
	vector_of_strings found_files=dir.getFiles();
	vector_of_strings expected_files;
	expected_files.push_back("B.txt");
	expected_files.push_back("C/C.txt");
	expected_files.push_back("C/C/C.txt");
	expected_files.push_back("D/C.txt");
	expected_files.push_back("A.txt");
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
	expected_files.push_back("C/C/C.txt");
	expected_files.push_back("D/C.txt");
	expected_files.push_back("A.txt");
	expected_files.push_back("B.txt");
	expected_files.push_back("C/C.txt");
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
	expected_block.push_back("B.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 4th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("D/C.txt");
	EXPECT_EQ(expected_block,block);

	// 5th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
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
	expected_block.push_back("B.txt");
	expected_block.push_back("C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C/C.txt");
	expected_block.push_back("D/C.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
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
	expected_block.push_back("B.txt");
	expected_block.push_back("C/C.txt");
	expected_block.push_back("C/C/C.txt");
	expected_block.push_back("D/C.txt");
	expected_block.push_back("A.txt");
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
	expected_block.push_back("B.txt");
	expected_block.push_back("C/C.txt");
	expected_block.push_back("C/C/C.txt");
	expected_block.push_back("D/C.txt");
	expected_block.push_back("A.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(9);

}

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

TEST_F(ChdbTest,usingFsExternalCommand) {

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

	string f,cmd;

	// create output directory
	dir.makeOutputDir();

	// output files
	vector_of_strings output_files;

	cmd = "./coucou.sh";
	EXPECT_THROW(dir.executeExternalCommand(cmd,output_files),runtime_error);
	
	f = "/B.txt";
	cmd = "./ext_cmd.sh " + prms.getInDir() + f + " " + prms.getOutDir() + f;
	EXPECT_EQ(0,dir.executeExternalCommand(cmd,output_files));

	f = "/D/C.txt";
	output_files.push_back(prms.getOutDir() + f);
	cmd = "./ext_cmd.sh " + prms.getInDir() + f + " " + prms.getOutDir() + f;
	EXPECT_EQ(1,dir.executeExternalCommand(cmd,output_files));

	FREE_ARGV(7);

	cmd = "rm -r " +  prms.getOutDir();
	system(cmd.c_str());
}

