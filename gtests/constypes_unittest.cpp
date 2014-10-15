
#include <errno.h>
#include "constypes_unittest.hpp"
#include <fstream>
using namespace std;


void ChdbTest::createFile(const string& d, const naco& n) {
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

string ChdbTest::readFile(const string & f) {
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

bool ChdbTest::existsFile(const string &f) {
	ifstream in(f.c_str());
	return in;
}

ChdbTest::ChdbTest(): input_dir("inputdir") {

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
	expected_file_pathes.push_back("B.txt");
	expected_file_pathes.push_back("C/C.txt");
	expected_file_pathes.push_back("C/C/C.txt");
	expected_file_pathes.push_back("D/C.txt");
	expected_file_pathes.push_back("A.txt");

	// The content
	expected_file_contents["B.txt"]     = "STS\t0\nTXT\tABCDEFGHIJKLMNO\n\n";
	expected_file_contents["C/C.txt"]   = "STS\t0\nTXT\tABCDEFGHIJKLMNOPQ\n\n";
	expected_file_contents["C/C/C.txt"] = "STS\t0\nTXT\tC\n\n";
	expected_file_contents["D/C.txt"]   = "STS\t1\nTXT\tABC\n\n";
	expected_file_contents["A.txt"]     = "STS\t0\nTXT\tABCDEF\n\n";
}
