
/**
   unit tests for the class Scheduler, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../system.hpp"
#include "../basicscheduler.hpp"
// the following to know if OUTDIRPERSLAVE is defined
#include "../usingfs.hpp"
#include <fstream>
using namespace std;

using ::testing::TestWithParam;
using ::testing::Values;

// Test basicscheduler::readFrmRecvBfr and basicscheduler::writeToSndBfr
TEST_F(SchedTestStrInt,readwriteToSndBfr) {

	// Init prms
	char* argv[11];
	INIT_ARGV(0,"scheduler_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"5");
	INIT_ARGV(9,"--out-files");
	INIT_ARGV(10,"%out-dir%/%path%");
	
	Parameters prms(11,argv);
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

	FREE_ARGV(11);
};


// Test ExecuteCommand (ie create slave output and tmp directory, execute the command on a block - With Tmp
// AN ERROR WILL BE GENERATED BY THE EXTERNAL COMMAND (see file D/C.txt)

TEST_P(TestCase1,AbortOnError) {
	cout << GetParam()->getDescription() << '\n';

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"scheduler_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--tmp-dir");
	INIT_ARGV(12,GetParam()->getTmpDir().c_str());

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

	Parameters prms(13,argv);
	string output_dir; 
	string output_root;

	// Inside a block to consolidate at end (see the destructor of dir)
	{
		auto_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
		Directories& dir = *aptr_dir.get();
		
		// Read the files
		dir.readFiles();
		
		// Create the output directory, removing it if already exists
		dir.makeOutDir(false,true);
		dir.makeTempOutDir();
		if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
			output_dir = getInputDir();
			output_dir.erase(output_dir.length()-3); // drop .db
			output_dir += ".out.db";
		} else {
			output_dir = getInputDir() + ".out";
		}
		EXPECT_EQ(output_dir,dir.getOutDir());

		output_root = dir.getOutDir();
		output_root = output_root.substr(0,output_root.length()-3);// drop the final .db

		cout << "================\n";
#ifdef NOTMP
		cout << "=== WARNING === NOTMP IS DEFINED ! ===\n";
#endif
		cout << "INPUT DIRECTORY            = " << prms.getInDir() << '\n';
		cout << "INPUT TEMPORARY DIRECTORY  = " << dir.getTempInDir() << '\n';
		cout << "OUTPUT  DIRECTORY          = " << dir.getOutDir() << "\n";
		if (  GetParam()->getDirectoryType() == "UsingBdbh" ) {
			cout << "OUTPUT         ROOT        = " << output_root << "\n";
		}
		cout << "OUTPUT TEMPORARY DIRECTORY = " << dir.getTempOutDir() << "\n";
		cout << "================\n";
		cerr << "THE FOLLOWING ERROR MESSAGE IS NORMAL\n";

		// execute command, initializing return_values and file_pathes
		BasicScheduler sched(prms,dir);
		sched.return_values.clear();
		sched.file_pathes = dir.nextBlock();
		EXPECT_NO_THROW(sched.executeCommand());

		// There is an error (file D/C.txt), errorHandle should find it and thus return true
		ofstream err;
		EXPECT_EQ(true,sched.errorHandle(err));
	}

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string slave_outdir = output_dir + ".0";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(slave_outdir,output_root + "/B.txt"));
	} else {
#ifdef OUTDIRPERSLAVE
		string slave_outdir = output_dir + ".0";
#else
		string slave_outdir = output_dir;
#endif
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(slave_outdir+"/B.txt"));
	}

	FREE_ARGV(13);
};

// Test ExecuteCommand, generating an error.txt error file
TEST_P(TestCase1,ContinueOnError) {
	cout << GetParam()->getDescription() << '\n';

	// Init prms
	char* argv[15];
	INIT_ARGV(0,"scheduler_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--tmp-dir");
	INIT_ARGV(12,GetParam()->getTmpDir().c_str());
	INIT_ARGV(13,"--on-error");
	INIT_ARGV(14,"errors.txt");

	Parameters prms(15,argv);

	string output_dir;
	string output_root;
	{
		auto_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
		Directories& dir = *aptr_dir.get();

		BasicScheduler sched(prms,dir);
		dir.makeOutDir(false,true);

		if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
			output_dir = getInputDir();
			output_dir.erase(output_dir.length()-3); // drop .db
			output_dir += ".out.db";
		} else {
			output_dir = getInputDir() + ".out";
		}
		EXPECT_EQ(output_dir,dir.getOutDir());

		output_root = dir.getOutDir();
		output_root = output_root.substr(0,output_root.length()-3);// drop the final .db

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

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string slave_outdir = output_dir + ".0";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(slave_outdir,output_root + "/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFileFromBdbh(slave_outdir,output_root + "/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(slave_outdir,output_root + "/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(slave_outdir,output_root + "/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(slave_outdir,output_root + "/A.txt"));
	} else {
#ifdef OUTDIRPERSLAVE
		string slave_outdir = output_dir + ".0";
#else
		string slave_outdir = output_dir;
#endif
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(slave_outdir+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFile(slave_outdir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(slave_outdir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(slave_outdir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(slave_outdir+"/A.txt"));
	}
	EXPECT_EQ("1\tD/C.txt\n\n",readFile("errors.txt"));

	FREE_ARGV(15);

	// Remove errors.txt
	system("rm errors.txt");
};

// Test ExecuteCommand, using the previously generated error file as input
TEST_P(TestCase1,ExecuteCommandFrmList1) {
	cout << GetParam()->getDescription() << '\n';

	// Init prms
	char* argv[15];
	INIT_ARGV(0,"scheduler_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"errors.txt");
	INIT_ARGV(13,"--tmp-dir");
	INIT_ARGV(14,GetParam()->getTmpDir().c_str());

	string current = ".";

	// Recreate the file errors.txt (removed at end of tests)
	createFile(current,naco("errors.txt","1\tD/C.txt"));

	string output_dir;
	string output_root;
	Parameters prms(15,argv);
	{
		auto_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
		Directories& dir = *aptr_dir.get();

		dir.makeOutDir(false,true);
		if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
			output_dir = getInputDir();
			output_dir.erase(output_dir.length()-3); // drop .db
			output_dir += ".out.db";
		} else {
			output_dir = getInputDir() + ".out";
		}
		EXPECT_EQ(output_dir,dir.getOutDir());

		output_root = dir.getOutDir();
		output_root = output_root.substr(0,output_root.length()-3);// drop the final .db

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
	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string slave_outdir = output_dir + ".0";
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(slave_outdir,output_root + "/D/C.txt"));
	} else {
#ifdef OUTDIRPERSLAVE
		string slave_outdir = output_dir + ".0";
#else
		string slave_outdir = output_dir;
#endif
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(slave_outdir+"/D/C.txt"));
	}

	// Remove errors.txt
	system("rm errors.txt");

	FREE_ARGV(15);
};

// Test ExecuteCommand, using a "One column file" as input
TEST_P(TestCase1,ExecuteCommandFrmList2) {
	cout << GetParam()->getDescription() << '\n';

	// Init prms
	char* argv[15];
	INIT_ARGV(0,"scheduler_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,getInputDir().c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"files.txt");
	INIT_ARGV(13,"--tmp-dir");
	INIT_ARGV(14,GetParam()->getTmpDir().c_str());


	string output_dir;
	string output_root;
	Parameters prms(15,argv);
	{
		auto_ptr<Directories> aptr_dir(GetParam()->createDirectory(prms));
		Directories& dir = *aptr_dir.get();

		dir.makeOutDir(false,true);
		if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
			output_dir = getInputDir();
			output_dir.erase(output_dir.length()-3); // drop .db
			output_dir += ".out.db";
		} else {
			output_dir = getInputDir() + ".out";
		}
		EXPECT_EQ(output_dir,dir.getOutDir());

		output_root = dir.getOutDir();
		output_root = output_root.substr(0,output_root.length()-3);// drop the final .db

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

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string slave_outdir = output_dir + ".0";
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(slave_outdir,output_root + "/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(slave_outdir,output_root + "/A.txt"));
	} else {
#ifdef OUTDIRPERSLAVE
		string slave_outdir = output_dir + ".0";
#else
		string slave_outdir = output_dir;
#endif
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(slave_outdir + "/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(slave_outdir + "/A.txt"));
	}
	
	// Remove files.txt
	system("rm files.txt");

	FREE_ARGV(15);
};

// Calling "none" for tmp-dir is just a trick: "none" does not exist, so there is NO tmp-dir
// If you do NOT specify --tmp-dir, you get the default tmpdir, which may - or not - be defined
// See parameters.cpp
// If you create "none" with "mkdir none" you are no more "NoTmp" and the test fails !

auto_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_notmp   (new ChdbTestsWithParamsUsingFs("none"));
auto_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_withtmp (new ChdbTestsWithParamsUsingFs("."));
auto_ptr<ChdbTestsWithParamsUsingBdbh> test_case_Bdbh_withtmp (new ChdbTestsWithParamsUsingBdbh("."));
	
INSTANTIATE_TEST_CASE_P(
	tmpOrNotSeveralDirectories,
	TestCase1,
//	Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get())
//	Values(test_case_Bdbh_withtmp.get())
	Values(test_case_Fs_notmp.get(),test_case_Bdbh_withtmp.get())
);

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

