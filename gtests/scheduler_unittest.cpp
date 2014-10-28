
/**
   unit tests for the class Scheduler, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../system.hpp"
#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;

TEST_F(SchedTestStrInt,readwriteToSndBfr) {

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

	// Load (empty) values and file_pathes from expected_bfr
	BasicScheduler sched(prms,dir);
	sched.readFrmRecvBfr(expected_bfr.c_str());
	EXPECT_EQ(0,sched.return_values.size());
	EXPECT_EQ(expected_file_pathes,sched.file_pathes);

	// Create a bfr from the (empty) values and file_pathes and compare it to expected_bfr
	size_t data_size;
	size_t bfr_size = expected_bfr.length() + 50;
	void * bfr = malloc(bfr_size);
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr.length());
	
	// clear file_pathes and reload values and file_pathes from  expected_bfr_1
	sched.file_pathes.clear();
	sched.readFrmRecvBfr(expected_bfr_1.c_str());
	EXPECT_EQ(expected_values_1,sched.return_values);
	EXPECT_EQ(expected_file_pathes,sched.file_pathes);

	// write again to bfr and compare value to expected_bfr_1
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr_1.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr_1.length());

	FREE_ARGV(9);
};

TEST_F(ChdbTest1,ExecuteCommandNoTmp) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");

	Parameters prms(11,argv);
	
	// Inside a block to consolidate at end (see the destructor of dir)
	{
		UsingFs dir(prms);
		
		// Read the files
		dir.readFiles();
		
		// Create the output directory, removing it if already exists
		dir.makeOutDir(false,true);
		dir.makeTempOutDir();
		
		// execute command, initializing return_values and file_pathes
		// An exception is generated for the file D/C.txt
			
		BasicScheduler sched(prms,dir);
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();
		EXPECT_NO_THROW(sched.executeCommand());
		ofstream err;
		EXPECT_EQ(true,sched.errorHandle(err));
	}
	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	system("rm -r inputdir.out");
	
	FREE_ARGV(11);
};

TEST_F(ChdbTest1,ExecuteCommandWithTmp) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--tmp-dir");
	INIT_ARGV(12,".");

/*
		{
			pid_t pid = getpid();
			ostringstream s_tmp;
			s_tmp << "gdbserver host:2345 --attach " << pid << "&";
			cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
			system(s_tmp.str().c_str());
			sleep(5);
		}
*/

	Parameters prms(11,argv);

	// Inside a block to consolidate at end (see the destructor of dir)
	{
		UsingFs dir(prms);
		
		// Read the files
		dir.readFiles();
		
		// Create the output directory, removing it if already exists
		dir.makeOutDir(false,true);
		dir.makeTempOutDir();
	
		// execute command, initializing return_values and file_pathes
		// An exception is generated for the file D/C.txt
			
		BasicScheduler sched(prms,dir);
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();
		EXPECT_NO_THROW(sched.executeCommand());
		ofstream err;
		EXPECT_EQ(true,sched.errorHandle(err));
	}
	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	system("rm -r inputdir.out");
	
	FREE_ARGV(13);
};

TEST_F(ChdbTest1,ExecuteCommandWithErr) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--on-error");
	INIT_ARGV(12,"errors.txt");

	Parameters prms(13,argv);

	{
		UsingFs dir(prms);
		BasicScheduler sched(prms,dir);
		dir.makeOutDir(false,true);
		/*
		{
			pid_t pid = getpid();
			ostringstream s_tmp;
			s_tmp << "gdbserver host:2345 --attach " << pid << "&";
			cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
			system(s_tmp.str().c_str());
			sleep(5);
		}
		*/
		// Read the files
		dir.readFiles();

		// execute command, initializing return_values and file_pathes
		// no exception, thanks to --on-error
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();

//		sched.executeCommand();
		EXPECT_NO_THROW(sched.executeCommand());

		// call errorHandle, generating errors.txt
		string e_out = prms.getErrFile();
		{
			ofstream e(e_out.c_str());
			sched.errorHandle(e);
			e.close();
		}

		ifstream e(e_out.c_str());
		EXPECT_EQ(true,e.good());
	}
	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));

	FREE_ARGV(13);
};

TEST_F(ChdbTest1,ExecuteCommandFrmList1) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"errors.txt");

	Parameters prms(13,argv);
	{
		UsingFs dir(prms);
		dir.makeOutDir(false,true);

		// Read the files
		dir.readFiles();

		// execute command, initializing return_values and file_pathes
		// The files used are read from errors.txt, generated by the test, ExecuteCommandWithErr
		// This test should have been run juste before
		// no exception, thanks to the last parameter of ext_cmd.sh
		BasicScheduler sched(prms,dir);
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();
		EXPECT_NO_THROW(sched.executeCommand());

		// only ONE result
		EXPECT_EQ(1,sched.return_values.size());
		EXPECT_EQ(1,sched.file_pathes.size());
	}
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));

	// Remove errors.txt
	system("rm errors.txt");
		
	FREE_ARGV(13);
};

TEST_F(ChdbTest1,ExecuteCommandFrmList2) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"files.txt");

	Parameters prms(13,argv);

	{
		UsingFs dir(prms);
		dir.makeOutDir(false,true);

		// Prepare files.txt
		ofstream f("files.txt");
		f << "C/C/C.txt\n";
		f << "# some comment\n";
		f << "A.txt\n";
		f.close();
		
		// Read the files
		dir.readFiles();

		// execute command, initializing return_values and file_pathes
		// The files used are read from files.txt (2 files only)
		BasicScheduler sched(prms,dir);
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();
		sched.executeCommand();
		EXPECT_NO_THROW(sched.executeCommand());

		// only TWO results
		EXPECT_EQ(2,sched.return_values.size());
		EXPECT_EQ(2,sched.file_pathes.size());
	}

	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));

	// Remove files.txt
	system("rm files.txt");

	FREE_ARGV(13);
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

