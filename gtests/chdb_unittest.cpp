
/**
   "unit tests" calling chdb by itself
*/


#include "constypes_unittest.hpp"
#include "../system.hpp"
//#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;
	
using ::testing::TestWithParam;
using ::testing::Values;

// One slave, 5 files, blocks of 1 file
TEST_P(TestCase1,Block1) {
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd_with_rank.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents_with_rank["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents_with_rank["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["A.txt"],readFile(output_dir+"/A.txt"));
	}
};

// Same thing, but specify a work directory for the command
// Warning, when executing the command, the names are now relative to work directory !
// Thus, we prefer to use absolute names for executable name and for input
// relative names for output is ok
// We also use here the --create-environment switch
TEST_P(TestCase1,Block1W) {
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	string wd;
	naco readme("README.txt","CREATED BY chdb SNIPPET");
	
	getCurrentDirName(wd);
	createFile(wd,readme);

	string cexe  = wd + "/ext_cmd_with_rank.sh ";
	string cindir= wd + "/%in-dir%/%path% ";
	cmd += "--command-line '";
	cmd += cexe;
	cmd += cindir;
	cmd += " %path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/OUT/%basename%/%path% ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir(); cmd += " ";
	cmd += "--work-dir ";
	cmd += "OUT/%basename% ";
	cmd += "--create-environment ";
	cmd += "'cp ../../../README.txt .'";
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	string output_dir = getInputDir() + ".out";
	EXPECT_EQ(expected_file_contents_with_rank["B.txt"],readFile(output_dir+"/OUT/B/B.txt"));
	EXPECT_EQ(expected_file_contents_with_rank["C/C.txt"],readFile(output_dir+"/OUT/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents_with_rank["C/C/C.txt"],readFile(output_dir+"/OUT/C/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents_with_rank["D/C.txt"],readFile(output_dir+"/OUT/C/D/C.txt"));
	EXPECT_EQ(expected_file_contents_with_rank["A.txt"],readFile(output_dir+"/OUT/A/A.txt"));
	EXPECT_EQ("CREATED BY chdb SNIPPET\n\n",readFile(output_dir+"/OUT/B/README.txt"));
	EXPECT_EQ("CREATED BY chdb SNIPPET\n\n",readFile(output_dir+"/OUT/C/README.txt"));
	EXPECT_EQ("CREATED BY chdb SNIPPET\n\n",readFile(output_dir+"/OUT/A/README.txt"));
};




// One slave, blocks of 2 files, no error generated
TEST_P(TestCase1,Block2) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd_with_rank.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--block-size 2 ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents_with_rank["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents_with_rank["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents_with_rank["A.txt"],readFile(output_dir+"/A.txt"));
	}
};

// One slave, One block of 5 files, no error generated
TEST_P(TestCase1,Block5) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--block-size 5 ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(output_dir+"/A.txt"));
	}
};

// An error is generated at first file, but it is trapped to the file errors.txt
TEST_P(TestCase1,onerror) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--on-error errors.txt ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should NOT have the word ABORTING in it
	EXPECT_NE(0,callSystem("grep -q ^ABORTING stdoe"));

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(output_dir+"/A.txt"));
	}
	EXPECT_EQ("1\tD/C.txt\n\n",readFile("errors.txt"));
};

// Using the errors.txt file created at previous test and limiting treatment to this file
// NOTE - The file is RECREATED because we are not sure of the tests execution order
TEST_P(TestCase1,onefile) {

	naco err("errors.txt","1\tD/C.txt\n\n");
	createFile(".",err);
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 2 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--in-files errors.txt ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());

	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	// Only ONE file created !
	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(false,existsFile(output_dir+"/B.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/A.txt"));
	}
};

// two slaves, block size 3, no error
TEST_P(TestCase1,twoslaves_blk3) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 3 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--block-size 3 ";
	cmd += "--sort-by-size ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(output_dir+"/A.txt"));
	}
};

// 5 files, 5 slaves, no error generated
TEST_P(TestCase1,fiveslaves) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 5 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);
	EXPECT_NE(0,callSystem("grep -q '^ERROR' stdoe"));
	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["B.txt"],readFileFromBdbh(output_dir,output_root+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFileFromBdbh(output_dir,output_root+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFileFromBdbh(output_dir,output_root+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFileFromBdbh(output_dir,output_root+"/A.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["B.txt"],readFile(output_dir+"/B.txt"));
		EXPECT_EQ(expected_file_contents["C/C.txt"],readFile(output_dir+"/C/C.txt"));
		EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile(output_dir+"/C/C/C.txt"));
		EXPECT_EQ(expected_file_contents["D/C.txt"],readFile(output_dir+"/D/C.txt"));
		EXPECT_EQ(expected_file_contents["A.txt"],readFile(output_dir+"/A.txt"));
	}
};

// Trying 5 files, blocks of 1 files, 10 slaves = should refuse to start without creating inputdir.out
TEST_P(TestCase1,tenslaves) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 10 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--out-file %out-dir%/%path% ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_NE(0,rvl);
	EXPECT_EQ(0,callSystem("grep -q 'ERROR - You should NOT use more than 5 slaves' stdoe"));
};

// Trying 10 files, blocks of 2 files, 2 slaves, file 0 is in error
// The blocks 0, 1, maybe 2 are treated, the blocks 4 & 5 should not be
TEST_P(TestCase2,errBlock2Slaves2) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 3 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--out-files %out-dir%/%path% ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--block-size 2 ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir();
	cmd += " >stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should have the word ABORTING in it
	EXPECT_EQ(0,callSystem("grep -q ^ABORTING stdoe"));

	// files created = 0 1 2 3

	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["0.txt"],readFileFromBdbh(output_dir,output_root+"/0.txt"));
		EXPECT_EQ(expected_file_contents["1.txt"],readFileFromBdbh(output_dir,output_root+"/1.txt"));
		EXPECT_EQ(expected_file_contents["2.txt"],readFileFromBdbh(output_dir,output_root+"/2.txt"));
		EXPECT_EQ(expected_file_contents["3.txt"],readFileFromBdbh(output_dir,output_root+"/3.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["0.txt"],readFile(output_dir+"/0.txt"));
		EXPECT_EQ(expected_file_contents["1.txt"],readFile(output_dir+"/1.txt"));
		EXPECT_EQ(expected_file_contents["2.txt"],readFile(output_dir+"/2.txt"));
		EXPECT_EQ(expected_file_contents["3.txt"],readFile(output_dir+"/3.txt"));
	}


	// may be created, may be not (depends on timing)
//	EXPECT_THROW(callSystem(cmd+"/4.txt",true),runtime_error);
//	EXPECT_THROW(callSystem(cmd+"/5.txt",true),runtime_error);

	// should not be created because of interruption
	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/6.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/7.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/8.txt"));
		EXPECT_EQ(false,existsFileFromBdbh(output_dir,output_root+"/9.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(false,existsFile(output_dir+"/6.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/7.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/8.txt"));
		EXPECT_EQ(false,existsFile(output_dir+"/9.txt"));
	}
};

// Trying 10 files, blocks of 2 files, 2 slaves, file 9 is in error
// Every file should be created, as the error happens quite at the end
TEST_P(TestCase3,errBlock2Slaves2) {
	string output_dir = getInputDir() + ".out";
	string cmd = "mpirun -n 3 ../chdb.exe --verbose ";
	cmd += "--command-line './ext_cmd.sh %in-dir%/%path% %out-dir%/%path%' ";
	cmd += "--in-type txt ";
	cmd += "--in-dir "; cmd += getInputDir(); cmd += " ";
	cmd += "--block-size 2 ";
	cmd += "--tmp-dir " + GetParam()->getTmpDir(); cmd += " ";
	cmd += "--out-files %out-dir%/%path% ";
	cmd += ">stdoe 2>&1";

	cerr << "NOW CALLING " << cmd << '\n';
	int rvl=system(cmd.c_str());
	EXPECT_EQ(0,rvl);

	// stderr should have the word ABORTING in it
	EXPECT_EQ(0,callSystem("grep -q ^ABORTING stdoe"));

	// files created = 1 2 3
	if ( GetParam()->getDirectoryType() == "UsingBdbh" ) {
		string output_dir = getInputDir();
		output_dir = output_dir.substr(0,output_dir.length()-3);
		string output_root = output_dir + ".out";
		output_dir += ".out.db";
		EXPECT_EQ(expected_file_contents["0.txt"],readFileFromBdbh(output_dir,output_root+"/0.txt"));
		EXPECT_EQ(expected_file_contents["1.txt"],readFileFromBdbh(output_dir,output_root+"/1.txt"));
		EXPECT_EQ(expected_file_contents["2.txt"],readFileFromBdbh(output_dir,output_root+"/2.txt"));
		EXPECT_EQ(expected_file_contents["3.txt"],readFileFromBdbh(output_dir,output_root+"/3.txt"));
		EXPECT_EQ(expected_file_contents["4.txt"],readFileFromBdbh(output_dir,output_root+"/4.txt"));
		EXPECT_EQ(expected_file_contents["5.txt"],readFileFromBdbh(output_dir,output_root+"/5.txt"));
		EXPECT_EQ(expected_file_contents["6.txt"],readFileFromBdbh(output_dir,output_root+"/6.txt"));
		EXPECT_EQ(expected_file_contents["7.txt"],readFileFromBdbh(output_dir,output_root+"/7.txt"));
		EXPECT_EQ(expected_file_contents["8.txt"],readFileFromBdbh(output_dir,output_root+"/8.txt"));
		EXPECT_EQ(expected_file_contents["9.txt"],readFileFromBdbh(output_dir,output_root+"/9.txt"));
	} else {
		string output_dir = getInputDir() + ".out";
		EXPECT_EQ(expected_file_contents["0.txt"],readFile(output_dir+"/0.txt"));
		EXPECT_EQ(expected_file_contents["1.txt"],readFile(output_dir+"/1.txt"));
		EXPECT_EQ(expected_file_contents["2.txt"],readFile(output_dir+"/2.txt"));
		EXPECT_EQ(expected_file_contents["3.txt"],readFile(output_dir+"/3.txt"));
		EXPECT_EQ(expected_file_contents["4.txt"],readFile(output_dir+"/4.txt"));
		EXPECT_EQ(expected_file_contents["5.txt"],readFile(output_dir+"/5.txt"));
		EXPECT_EQ(expected_file_contents["6.txt"],readFile(output_dir+"/6.txt"));
		EXPECT_EQ(expected_file_contents["7.txt"],readFile(output_dir+"/7.txt"));
		EXPECT_EQ(expected_file_contents["8.txt"],readFile(output_dir+"/8.txt"));
		EXPECT_EQ(expected_file_contents["9.txt"],readFile(output_dir+"/9.txt"));
	}
};

// Calling "none" for tmp-dir is just a trick: "none" does not exist, so there is NO tmp-dir
// If you do NOT specify --tmp-dir, you get the default tmpdir, which may - or not - be defined
// See parameters.cpp
// If you create "none" with "mkdir none" you are no more "NoTmp" and the test fails !

auto_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_notmp   (new ChdbTestsWithParamsUsingFs("none"));
auto_ptr<ChdbTestsWithParamsUsingFs> test_case_Fs_withtmp (new ChdbTestsWithParamsUsingFs("."));
//auto_ptr<ChdbTestsWithParamsUsingBdbh> test_case_Bdbh_withtmp (new ChdbTestsWithParamsUsingBdbh("."));


INSTANTIATE_TEST_CASE_P(
	tmpOrNotSeveralDirectories,
	TestCase1,
	Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get())
	//Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get());
);
INSTANTIATE_TEST_CASE_P(
	tmpOrNotSeveralDirectories,
	TestCase2,
	Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get());
	//Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get());
);
INSTANTIATE_TEST_CASE_P(
	tmpOrNotSeveralDirectories,
	TestCase3,
	Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get());
	//Values(test_case_Fs_notmp.get(),test_case_Fs_withtmp.get(),test_case_Bdbh_withtmp.get());
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
