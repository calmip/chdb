
/**
   unit tests for the class Directories, and its subclasses
*/

#include "../constypes.hpp"
#include "constypes_unittest.hpp"
#include "../system.hpp"
#include "../usingfs.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
using namespace std;

#define OUTDIR "inputdir.out"
#ifdef OUTDIRPERSLAVE
#define SLAVEOUTDIR "inputdir.out.1"
#else   
#define SLAVEOUTDIR OUTDIR
#endif

// testing UsingFs::makeOutDir
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

	// create inputdir.out
	EXPECT_NO_THROW(dir.makeOutDir(false,true));

	// replace inputdir.out
	EXPECT_NO_THROW(dir.makeOutDir(false,true));

	// could not create inputdir.out (already exists)
	EXPECT_THROW(dir.makeOutDir(false,false),runtime_error);

	// remove inputdir.out
	string cmd="rm -r " + prms.getOutDir();
	EXPECT_NO_THROW(callSystem(cmd,true));

	// Call with rank_flg=true
	dir.setRank(1,2);
	EXPECT_NO_THROW(dir.makeOutDir(true,true));
	cmd="rm -r " + prms.getOutDir();
	EXPECT_THROW(callSystem(cmd,true),runtime_error);

	FREE_ARGV(7);
}

// testing UsingFs::makeTempOutDir, No temporary
TEST(usingFs,usingFsMakeTempOutDirNoTmp) {
	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,"none");

	// Calling "none" tmp-dir is just a trick: "none" does not exist, so there is NO tmp-dir
	// If you do NOT specify --tmp-dir, you get the default tmpdir, which may - or not - be defined
	// See parameters.cpp
	// If you create "none" with "mkdir none" you are no more "NoTmp" and the test fails !

	string blank = "";
	string cmd = "ls -l ";
	string out = "inputdir.out";
	out += "_??????";
	cmd += out;

	Parameters prms(9,argv);
	UsingFs dir(prms);
	dir.makeOutDir(false,true);
	dir.makeTempOutDir();
	EXPECT_EQ(prms.getOutDir(),dir.getTempOutDir());
	EXPECT_THROW(callSystem(cmd,true),runtime_error);
	system("rm -r inputdir.out*");

	FREE_ARGV(9);
}

// testing UsingFs::makeTempOutDir, with temporary
TEST(usingFs,usingFsMakeTempOutDirWithTmp) {
	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,"inputdir");
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,".");

	string blank = "";
	string cmd = "ls -l ";
	string out = OUTDIR;
	out += "_??????";
	cmd += out;

	Parameters prms(9,argv);
	UsingFs dir(prms);
	dir.makeOutDir(false,true);
	dir.makeTempOutDir();
	string tmp_out = dir.getTempOutDir();
#ifdef NOTMP
	EXPECT_EQ(OUTDIR,tmp_out);
	EXPECT_THROW(callSystem(cmd,true),runtime_error);
#else
	EXPECT_NE(blank,tmp_out);
	EXPECT_NO_THROW(callSystem(cmd,true));
	string cmd1="rmdir ";
	cmd1 += tmp_out;
	EXPECT_NO_THROW(callSystem(cmd1,true));
#endif
	
	system("rm -r inputdir.out*");

	FREE_ARGV(9);
}

// testing UsingFs::findOfCreateDir (subdirectories of output dir)
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

// testing UsingFs::getFiles, unsorted (ie sorted in their names)
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
	expected_files.push_back("A.txt");
	expected_files.push_back("B.txt");
	expected_files.push_back("C/C.txt");
	expected_files.push_back("C/C/C.txt");
	expected_files.push_back("D/C.txt");
	EXPECT_EQ(expected_files,found_files);

	// again
	found_files=dir.getFiles();
	EXPECT_EQ(expected_files,found_files);

	FREE_ARGV(7);

}

// testing UsingFs::getFiles, sorted in size
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
	dir.setRank(0,4);
	vector_of_strings found_files=dir.getFiles();
	list<string> expected_files;
	expected_files.push_back("C/C/C.txt");
	expected_files.push_back("D/C.txt");
	expected_files.push_back("A.txt");
	expected_files.push_back("B.txt");
	expected_files.push_back("C/C.txt");
	expected_files.reverse();
	EXPECT_EQ(true,equal(expected_files.begin(),expected_files.end(),found_files.begin()));

	// again
	found_files=dir.getFiles();
	EXPECT_EQ(true,equal(expected_files.begin(),expected_files.end(),found_files.begin()));

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

	int nb_files = dir.getNbOfFiles();
	EXPECT_EQ(5,nb_files);

	// Add a blank file name, it will be ignored !
	dir.files.push_back("");
	EXPECT_EQ(5,dir.getNbOfFiles());

	// 1th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("B.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 4th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 5th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("D/C.txt");
	EXPECT_EQ(expected_block,block);

	// 6th blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("");
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

	// Read the files
	dir.readFiles();

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
	expected_block.push_back("B.txt");
	EXPECT_EQ(expected_block,block);

	// 2nd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("C/C.txt");
	expected_block.push_back("C/C/C.txt");
	EXPECT_EQ(expected_block,block);

	// 3rd blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("D/C.txt");
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

	// Read the files
	dir.readFiles();

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
	expected_block.push_back("B.txt");
	expected_block.push_back("C/C.txt");
	expected_block.push_back("C/C/C.txt");
	expected_block.push_back("D/C.txt");
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

	// Read the files
	dir.readFiles();

	// 1st blck
	block = dir.nextBlock();
	expected_block.clear();
	expected_block.push_back("A.txt");
	expected_block.push_back("B.txt");
	expected_block.push_back("C/C.txt");
	expected_block.push_back("C/C/C.txt");
	expected_block.push_back("D/C.txt");
	EXPECT_EQ(expected_block,block);

	// next blcks: empty
	block = dir.nextBlock();
	expected_block.clear();
	EXPECT_EQ(expected_block,block);

	FREE_ARGV(9);

}

// testing UsingFs::getTempOutDir, no temporary directory !
TEST_F(ChdbTest,getTempOutDirNoTmp) {
	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,"none");

	Parameters prms(9,argv);
	UsingFs dir(prms);

	EXPECT_THROW(dir.getOutDir(),logic_error);
	dir.makeOutDir(false,true);
	EXPECT_NO_THROW(dir.getOutDir());
	EXPECT_EQ((string) OUTDIR, dir.getOutDir());

	EXPECT_THROW(dir.getTempOutDir(),logic_error);
	dir.makeTempOutDir();
	EXPECT_NO_THROW(dir.getTempOutDir());
	EXPECT_EQ((string) OUTDIR,dir.getTempOutDir());

	FREE_ARGV(9);
}

// testing UsingFs::getTempOutDir, with a temporary directory
TEST_F(ChdbTest,getTempOutDirWithTmp) {
	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,".");

	Parameters prms(9,argv);
	UsingFs dir(prms);

	// exception tempdir not yet initialized !
	EXPECT_THROW(dir.getTempOutDir(),logic_error);

	// exception outdir not yet initialized !
	EXPECT_THROW(dir.makeTempOutDir(),logic_error);
	EXPECT_THROW(dir.getOutDir(),logic_error);

	// making things correctly
	EXPECT_NO_THROW(dir.makeOutDir(false,true));
	EXPECT_NO_THROW(dir.makeTempOutDir());

	EXPECT_NO_THROW(dir.getTempOutDir());
	string tmpdir = dir.getTempOutDir();
	EXPECT_EQ((string) OUTDIR ,dir.getOutDir());

#ifdef NOTMP
	// NOTMP => The name of the tmp directory is ALSO the name of the output directory
	EXPECT_EQ((string) OUTDIR,tmpdir);
	EXPECT_NO_THROW(callSystem("rmdir inputdir.out"));
#else
	// NOTMP not defined => Other name
	EXPECT_EQ((string) "./inputdir.out_",tmpdir.substr(0,15));
	EXPECT_NO_THROW(callSystem("rmdir inputdir.out_??????"));
#endif

	FREE_ARGV(9);
}

// testing Directory::completeFilePath, no temporary directory
TEST_F(ChdbTest,completeFilePathNoTmp) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,"none");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);
	dir.setRank(1,2);
	dir.makeOutDir(true,true);
	dir.makeTempOutDir();

	string f1 = "A/B/C/D.txt";

	string s1 = "%out-dir%/%path%";
	string s1_1 = "%path%";
	string s1_2 = s1_1;
	string s2 = "%out-dir%/%name%";
	string s3 = "%out-dir%/%basename%.out";
	string s4 = "%out-dir%/%dirname%/%basename%.out";
	string s5 = s1 + "%%" + s1 + "%%" + s2 + "%%" + s2 + "%%" + s3 + "%%" + s3;
	dir.completeFilePath(f1,s1);

	// Testing the behaviour when %out-dir% is not specified
	dir.completeFilePath(f1,s1_1);
	dir.completeFilePath(f1,s1_2,true);

	dir.completeFilePath(f1,s2);
	dir.completeFilePath(f1,s3);
	dir.completeFilePath(f1,s4);
	dir.completeFilePath(f1,s5);

	string slave_outdir = SLAVEOUTDIR;
	string expected_s1 = slave_outdir + "/A/B/C/D.txt";
	string expected_s1_1="A/B/C/D.txt";
	string expected_s1_2 = expected_s1;
	string expected_s2 = slave_outdir + "/D.txt";
	string expected_s3 = slave_outdir + "/D.out";
	string expected_s4 = slave_outdir + "/A/B/C/D.out";
	string expected_s5 = expected_s1 + "%%" + expected_s1 + "%%" + expected_s2 + "%%" + expected_s2 + "%%" + expected_s3 + "%%" + expected_s3;
	vector_of_strings block;
	vector_of_strings expected_block;

	EXPECT_EQ(expected_s1,s1);
	EXPECT_EQ(expected_s1_1,s1_1);
	EXPECT_EQ(expected_s1_2,s1_2);
	EXPECT_EQ(expected_s2,s2);
	EXPECT_EQ(expected_s3,s3);
	EXPECT_EQ(expected_s4,s4);
	EXPECT_EQ(expected_s5,s5);

	FREE_ARGV(9);

}

// testing Directory::completeFilePath, with a temporary directory
TEST_F(ChdbTest,completeFilePathWithTmp) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--tmp-dir");
	INIT_ARGV(8,".");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);

	dir.setRank(1,2);
	dir.makeOutDir(true,true);
	dir.makeTempOutDir();

	string f1 = "A/B/C/D.txt";

	string s1 = "%out-dir%/%path%";
	string s2 = "%out-dir%/%name%";
	string s3 = "%out-dir%/%basename%.out";
	string s4 = "%out-dir%/%dirname%/%basename%.out";
	string s5 = s1 + "%%" + s1 + "%%" + s2 + "%%" + s2 + "%%" + s3 + "%%" + s3;
	dir.completeFilePath(f1,s1);
	dir.completeFilePath(f1,s2);
	dir.completeFilePath(f1,s3);
	dir.completeFilePath(f1,s4);
	dir.completeFilePath(f1,s5);

	string tmp = dir.getTempOutDir();

	string expected_s1 = tmp + "/A/B/C/D.txt";
	string expected_s2 = tmp + "/D.txt";
	string expected_s3 = tmp + "/D.out";
	string expected_s4 = tmp + "/A/B/C/D.out";
	string expected_s5 = expected_s1 + "%%" + expected_s1 + "%%" + expected_s2 + "%%" + expected_s2 + "%%" + expected_s3 + "%%" + expected_s3;
	vector_of_strings block;
	vector_of_strings expected_block;

	EXPECT_EQ(expected_s1,s1);
	EXPECT_EQ(expected_s2,s2);
	EXPECT_EQ(expected_s3,s3);
	EXPECT_EQ(expected_s4,s4);
	EXPECT_EQ(expected_s5,s5);

	FREE_ARGV(9);

}

// Testing usingFs::executeExternalCommand
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
	dir.makeOutDir(false,true);

	// output files
	vector_of_strings output_files;

	cmd = "./coucou.sh";
	int rvl=0;
	cerr << "10 LINES WILL BE WRITTEN TO STDERR - THIS IS NORMAL BEHAVIOUR\n";
	EXPECT_NO_THROW(rvl=dir.executeExternalCommand(cmd,output_files));
	EXPECT_EQ(127,rvl);
	
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

/** 
 * @brief Convenient function: converting from a table of ints to a vector of strings
 * 
 * @param tab 
 * @param s 
 * 
 * @return 
 */
vector_of_strings int2strings(int* tab,size_t s) {
	
	vector_of_strings rvl;
	ostringstream tmp;
	for (size_t i=0; i<s; i++) {
		tmp.str("");
		if (tab[i]>=0) {
			tmp << tab[i];
		};
		rvl.push_back(tmp.str());
	}
	return rvl;
}

void initFinfo(list<Finfo> & f_i, size_t s) {
	ostringstream tmp;
	off_t sze=5000;
	for (int i=s-1; i>-1; --i) {
		tmp.str("");
		tmp << i;
		f_i.push_back(Finfo(tmp.str(),sze));
		sze += 100;
	}
}

// testing UsingFs::buildBlocks
//         distribution in blocks size 5, from a sorted array, size 40
TEST_F(ChdbTest,usingFsSortFiles1) {
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
	dir.setRank(0,5);

	vector_of_strings files;
	list<Finfo> f_info;
	initFinfo(f_info,40);
	dir.buildBlocks(f_info,files);

	int tmp[] = {0,4,8,12,16,1,5,9,13,17,2,6,10,14,18,3,7,11,15,19,20,24,28,32,36,21,25,29,33,37,22,26,30,34,38,23,27,31,35,39};
	vector_of_strings expected_files = int2strings(tmp,40);
	EXPECT_EQ(expected_files,files);

	FREE_ARGV(9);
}

// testing UsingFs::buildBlocks
//         distribution in blocks size 1, from a sorted array, size 16
TEST_F(ChdbTest,usingFsSortFiles3) {
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
	INIT_ARGV(8,"1");

	
	Parameters prms(9,argv);
	UsingFs dir(prms);
	dir.setRank(0,5);

	vector_of_strings files;
	list<Finfo> f_info;
	initFinfo(f_info,13);
	dir.buildBlocks(f_info,files);

	int tmp[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,-1,-1,-1};
	vector_of_strings expected_files = int2strings(tmp,16);
	EXPECT_EQ(expected_files,files);

	FREE_ARGV(9);
}

// testing UsingFs::buildBlocks
//         distribution in blocks size 5, from a sorted array, size 33
TEST_F(ChdbTest,usingFsSortFiles2) {
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
	dir.setRank(0,5);

	vector_of_strings files;
	list<Finfo> f_info;
	initFinfo(f_info,33);
	dir.buildBlocks(f_info,files);

	int tmp[] = {0,4,8,12,16,1,5,9,13,17,2,6,10,14,18,3,7,11,15,19,20,24,28,32,-1,21,25,29,-1,-1,22,26,30,-1,-1,23,27,31,-1,-1};
	vector_of_strings expected_files = int2strings(tmp,40);
	EXPECT_EQ(expected_files,files);

	FREE_ARGV(9);
}
