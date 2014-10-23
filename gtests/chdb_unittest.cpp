
/**
   unit tests for the class Directories, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../system.hpp"
//#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;
	
// One slave, 5 files, blocks of 1 file
TEST_F(ChdbTest1,Block1) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path%";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// One slave, blocks of 2 files, no error generated
TEST_F(ChdbTest1,Block2) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path%";
	cmd += "--block-size 2";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// One slave, One block of 5 files, no error generated
TEST_F(ChdbTest1,Block5) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path%";
	cmd += "--block-size 5";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// An error is generated at first file, but it is trapped to the file errors.txt
TEST_F(ChdbTest1,onerror) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--on-error errors.txt ";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should NOT have the word ABORTING in it
	EXPECT_NE(0,callSystem("grep -q ^ABORTING stdoe"));

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_EQ("1\tD/C.txt\n\n",readFile("errors.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// Using the errors.txt file created at previous test and limiting treatment to this file
TEST_F(ChdbTest1,onefile) {

    // The file errors.txt should have been created by previous test
	string in_dir = "inputdir";
	string cmd = "mpirun -n 2 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--in-files errors.txt ";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());

	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	// Only ONE file created !
	EXPECT_EQ(false,existsFile("inputdir.out/B.txt"));
	EXPECT_EQ(false,existsFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(false,existsFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(false,existsFile("inputdir.out/A.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// two slaves, block size 3, no error
TEST_F(ChdbTest1,twoslaves_blk3) {

	string in_dir = "inputdir";
	system("rm -r inputdir.out");

	string cmd = "mpirun -n 3 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--block-size 3 ";
	cmd += "--sort-by-size";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// 5 files, 5 slaves, no error generated
TEST_F(ChdbTest1,fiveslaves) {

	string in_dir = "inputdir";
	system("rm -r inputdir.out");

	string cmd = "mpirun -n 5 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path%";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));


	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));
	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// Trying 5 files, blocks of 1 files, 10 slaves = should refuse to start without creating inputdir.out
TEST_F(ChdbTest1,tenslaves) {

	string in_dir = "inputdir";

	string cmd = "mpirun -n 10 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += in_dir; cmd += " ";
	cmd += "--out-file %out-dir%/%path%";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_NE(0,rvl);
	EXPECT_EQ(0,callSystem("grep -q 'ERROR - You should NOT use more than 5 slaves' stdoe"));
	EXPECT_THROW(callSystem("rm -r /tmp/inputdir.out*",true),runtime_error);
};

// Trying 10 files, blocks of 2 files, 2 slaves, file 0 is in error
// The blocks 0, 1, maybe 2 are treated, the blocks 4 & 5 should not be
TEST_F(ChdbTest2,errBlock2Slaves2) {

	string cmd = "mpirun -n 3 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir inputdir ";
	cmd += "--block-size 2";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should have the word ABORTING in it
	EXPECT_EQ(0,callSystem("grep -q ^ABORTING stdoe"));

	// files created = 0 1 2 3
	EXPECT_EQ(expected_file_contents["0.txt"],readFile("inputdir.out/0.txt"));
	EXPECT_EQ(expected_file_contents["1.txt"],readFile("inputdir.out/1.txt"));
	EXPECT_EQ(expected_file_contents["2.txt"],readFile("inputdir.out/2.txt"));
	EXPECT_EQ(expected_file_contents["3.txt"],readFile("inputdir.out/3.txt"));
	// may be created, may be not (depends on timing)
//	EXPECT_THROW(callSystem("ls inputdir.out/4.txt",true),runtime_error);
//	EXPECT_THROW(callSystem("ls inputdir.out/5.txt",true),runtime_error);

	// should not be created because of interruption
	EXPECT_THROW(callSystem("ls inputdir.out/6.txt",true),runtime_error);
	EXPECT_THROW(callSystem("ls inputdir.out/7.txt",true),runtime_error);
	EXPECT_THROW(callSystem("ls inputdir.out/8.txt",true),runtime_error);
	EXPECT_THROW(callSystem("ls inputdir.out/9.txt",true),runtime_error);
};

// Trying 10 files, blocks of 2 files, 2 slaves, file 9 is in error
// Every file should be created, as the error happens quite at the end
TEST_F(ChdbTest3,errBlock2Slaves2) {

	string cmd = "mpirun -n 3 ../chdb --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir inputdir ";
	cmd += "--block-size 2";
	cmd += " >stdoe 2>&1";

	cout << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should have the word ABORTING in it
	EXPECT_EQ(0,callSystem("grep -q ^ABORTING stdoe"));

	// files created = 1 2 3
	EXPECT_EQ(expected_file_contents["0.txt"],readFile("inputdir.out/0.txt"));
	EXPECT_EQ(expected_file_contents["1.txt"],readFile("inputdir.out/1.txt"));
	EXPECT_EQ(expected_file_contents["2.txt"],readFile("inputdir.out/2.txt"));
	EXPECT_EQ(expected_file_contents["3.txt"],readFile("inputdir.out/3.txt"));
	EXPECT_EQ(expected_file_contents["4.txt"],readFile("inputdir.out/4.txt"));
	EXPECT_EQ(expected_file_contents["5.txt"],readFile("inputdir.out/5.txt"));
	EXPECT_EQ(expected_file_contents["6.txt"],readFile("inputdir.out/6.txt"));
	EXPECT_EQ(expected_file_contents["7.txt"],readFile("inputdir.out/7.txt"));
	EXPECT_EQ(expected_file_contents["8.txt"],readFile("inputdir.out/8.txt"));
	EXPECT_EQ(expected_file_contents["9.txt"],readFile("inputdir.out/9.txt"));
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

