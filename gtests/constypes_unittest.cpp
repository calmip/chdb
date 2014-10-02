
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
		throw(runtime_error(f.c_str()));
	}
	out << n.content << '\n';
}

ChdbTest::ChdbTest(): input_dir("inputdir") {
	// Create and populate inputdir
	vector<naco> files;

	// these files will be considered by Directories
	files.push_back(naco("A.txt","ABCDEF"));
	files.push_back(naco("B.txt","ABCDEFGHIJKLMNO"));
	files.push_back(naco("C/C.txt","ABCDEFGHIJKLMNOPQ"));
	files.push_back(naco("D/C.txt","ABC"));
	files.push_back(naco("C/C/C.txt","C"));
	
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
ChdbTest::~ChdbTest() {
	// remove inputdir
	string cmd = "rm -rf ";
	cmd += input_dir;
	system(cmd.c_str());
}
