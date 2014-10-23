
#include <errno.h>
#include "constypes_unittest.hpp"
#include <fstream>
using namespace std;


void createFile(const string& d, const naco& n) {
	string f = d;
	f += '/';
	f += n.name;
	ofstream out(f.c_str());
	if (!out) {
		string msg = "ERROR - Could not create the file ";
		msg += f;
		throw(runtime_error(msg));
	}
	out << n.content << '\n';
}

string readFile(const string & f) {
	ifstream in(f.c_str());
	if (!in) {
		string msg = "ERROR - Could not open the file ";
		msg += f;
		throw(runtime_error(msg));
	}
	string rvl;
	while(in) {
		string tmp;
		getline(in,tmp);
		rvl += tmp;
		rvl += '\n';
	}
	return rvl;
}

bool existsFile(const string &f) {
	ifstream in(f.c_str());
	return in;
}

// The standard test fixture: create inputdir
ChdbTest::ChdbTest(): input_dir("inputdir") {

	system("rm -r inputdir");
	// Nothing to do if inputdir already exists !
	struct stat buf;
	int err = stat("inputdir", &buf);
	if (err==-1 && errno==ENOENT) {
	
		// Create and populate inputdir
		vector<naco> files;
		
		// these files will be considered by Directories
		files.push_back(naco("B.txt","0\tABCDEFGHIJKLMNO"));
		files.push_back(naco("C/C.txt","0\tABCDEFGHIJKLMNOPQ"));
		files.push_back(naco("C/C/C.txt","0\tC"));
		files.push_back(naco("D/C.txt","1\tABC"));
		files.push_back(naco("A.txt","0\tABCDEF"));
	
		// these files will NOT be considered by Directories
		files.push_back(naco("Atxt","ABCDEFGHIJKLMNO"));
		files.push_back(naco("B.xt","ABCDEFGHIJKLMNO"));
		files.push_back(naco("B.ttxt","ABCDEFGHIJKLMNO"));
		files.push_back(naco("B.txtt","ABCDEFGHIJKLMNO"));
	
		// Create the hierarchy
		mkdir("inputdir",0700);
		mkdir("inputdir/C",0700);
		mkdir("inputdir/C/C",0700);
		mkdir("inputdir/D",0700);
	
		// Create the files
		for (size_t i=0;i<files.size();++i) {
			createFile(input_dir,files[i]);
		}
	}
	else if (err == -1) {
		throw(runtime_error("ERROR"));
	};

	// Expected created files in output directory
	expected_file_pathes.push_back("A.txt");
	expected_file_pathes.push_back("B.txt");
	expected_file_pathes.push_back("C/C.txt");
	expected_file_pathes.push_back("C/C/C.txt");
	expected_file_pathes.push_back("D/C.txt");

	// The content
	expected_file_contents["B.txt"]     = "STS\t0\nTXT\tABCDEFGHIJKLMNO\n\n";
	expected_file_contents["C/C.txt"]   = "STS\t0\nTXT\tABCDEFGHIJKLMNOPQ\n\n";
	expected_file_contents["C/C/C.txt"] = "STS\t0\nTXT\tC\n\n";
	expected_file_contents["D/C.txt"]   = "STS\t1\nTXT\tABC\n\n";
	expected_file_contents["A.txt"]     = "STS\t0\nTXT\tABCDEF\n\n";
}

// Another test fixture: rm previous inputdir and create a new one
// 10 files created, file 0 is in error
ChdbTest2::ChdbTest2(): input_dir("inputdir") {

	system("rm -r inputdir");

	// Nothing to do if inputdir already exists !
	struct stat buf;
	int err = stat("inputdir", &buf);
	if (err==-1 && errno==ENOENT) {
	
		// Create and populate inputdir
		vector<naco> files;
		
		// Create 10 files in the directories, the 1st will generate an error (see ext_cmd.sh)
		// these files will be considered by Directories
		files.push_back(naco("0.txt","1\t0"));
		files.push_back(naco("1.txt","0\t1"));
		files.push_back(naco("2.txt","0\t2"));
		files.push_back(naco("3.txt","0\t3"));
		files.push_back(naco("4.txt","0\t4"));
		files.push_back(naco("5.txt","0\t5"));
		files.push_back(naco("6.txt","0\t6"));
		files.push_back(naco("7.txt","0\t7"));
		files.push_back(naco("8.txt","0\t8"));
		files.push_back(naco("9.txt","0\t9"));
		
		// Create the hierarchy
		mkdir("inputdir",0700);
	
		// Create the files
		for (size_t i=0;i<files.size();++i) {
			createFile(input_dir,files[i]);
		}
	}
	else if (err == -1) {
		throw(runtime_error("ERROR"));
	};

	// Expected created files in output directory
	expected_file_pathes.push_back("0.txt");
	expected_file_pathes.push_back("1.txt");
	expected_file_pathes.push_back("2.txt");
	expected_file_pathes.push_back("3.txt");
	expected_file_pathes.push_back("4.txt");
	expected_file_pathes.push_back("5.txt");
	expected_file_pathes.push_back("6.txt");
	expected_file_pathes.push_back("7.txt");
	expected_file_pathes.push_back("8.txt");
	expected_file_pathes.push_back("9.txt");

	// The content
	expected_file_contents["0.txt"]     = "STS\t1\nTXT\t0\n\n";
	expected_file_contents["1.txt"]     = "STS\t0\nTXT\t1\n\n";
	expected_file_contents["2.txt"]     = "STS\t0\nTXT\t2\n\n";
	expected_file_contents["3.txt"]     = "STS\t0\nTXT\t3\n\n";
	expected_file_contents["4.txt"]     = "STS\t0\nTXT\t4\n\n";
	expected_file_contents["5.txt"]     = "STS\t0\nTXT\t5\n\n";
	expected_file_contents["6.txt"]     = "STS\t0\nTXT\t6\n\n";
	expected_file_contents["7.txt"]     = "STS\t0\nTXT\t7\n\n";
	expected_file_contents["8.txt"]     = "STS\t0\nTXT\t8\n\n";
	expected_file_contents["9.txt"]     = "STS\t0\nTXT\t9\n\n";
}

// Another test fixture: rm previous inputdir and create a new one
// 10 files created, file 9 is in error
ChdbTest3::ChdbTest3(): input_dir("inputdir") {

	system("rm -r inputdir");

	// Nothing to do if inputdir already exists !
	struct stat buf;
	int err = stat("inputdir", &buf);
	if (err==-1 && errno==ENOENT) {
	
		// Create and populate inputdir
		vector<naco> files;
		
		// Create 10 files in the directories, the last will generate an error (see ext_cmd.sh)
		// these files will be considered by Directories
		files.push_back(naco("0.txt","0\t0"));
		files.push_back(naco("1.txt","0\t1"));
		files.push_back(naco("2.txt","0\t2"));
		files.push_back(naco("3.txt","0\t3"));
		files.push_back(naco("4.txt","0\t4"));
		files.push_back(naco("5.txt","0\t5"));
		files.push_back(naco("6.txt","0\t6"));
		files.push_back(naco("7.txt","0\t7"));
		files.push_back(naco("8.txt","0\t8"));
		files.push_back(naco("9.txt","1\t9"));
		
		// Create the hierarchy
		mkdir("inputdir",0700);
	
		// Create the files
		for (size_t i=0;i<files.size();++i) {
			createFile(input_dir,files[i]);
		}
	}
	else if (err == -1) {
		throw(runtime_error("ERROR"));
	};

	// Expected created files in output directory
	expected_file_pathes.push_back("0.txt");
	expected_file_pathes.push_back("1.txt");
	expected_file_pathes.push_back("2.txt");
	expected_file_pathes.push_back("3.txt");
	expected_file_pathes.push_back("4.txt");
	expected_file_pathes.push_back("5.txt");
	expected_file_pathes.push_back("6.txt");
	expected_file_pathes.push_back("7.txt");
	expected_file_pathes.push_back("8.txt");
	expected_file_pathes.push_back("9.txt");

	// The content
	expected_file_contents["0.txt"]     = "STS\t0\nTXT\t0\n\n";
	expected_file_contents["1.txt"]     = "STS\t0\nTXT\t1\n\n";
	expected_file_contents["2.txt"]     = "STS\t0\nTXT\t2\n\n";
	expected_file_contents["3.txt"]     = "STS\t0\nTXT\t3\n\n";
	expected_file_contents["4.txt"]     = "STS\t0\nTXT\t4\n\n";
	expected_file_contents["5.txt"]     = "STS\t0\nTXT\t5\n\n";
	expected_file_contents["6.txt"]     = "STS\t0\nTXT\t6\n\n";
	expected_file_contents["7.txt"]     = "STS\t0\nTXT\t7\n\n";
	expected_file_contents["8.txt"]     = "STS\t0\nTXT\t8\n\n";
	expected_file_contents["9.txt"]     = "STS\t1\nTXT\t9\n\n";
}
