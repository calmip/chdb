
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
		//rvl += '\n';
	}
	return rvl;
}

string readFileFromBdbh(const string& db, const string& key) {
	string cmd = "./bdbh --database ";
	string tmp_out = "tmp.out";
	cmd += db;
	cmd += " cat ";
	cmd += key;
	cmd += " >";
	cmd += tmp_out;
	system(cmd.c_str());
	string out = readFile(tmp_out);
#ifdef REMOVE_OUTPUT
	unlink(tmp_out.c_str());
#endif
	return out;
}

bool existsFile(const string &f) {
	ifstream in(f.c_str());
	return in;
}

bool existsFileFromBdbh(const string& db, const string& key) {
	string cmd = "./bdbh --database ";
	cmd += db;
	cmd += " ls ";
	cmd += key;
	int rvl = system(cmd.c_str());
	return (rvl==0);
}
	
void removeFile(const string& f) {
#ifdef REMOVE_OUTPUT
	string cmd = "rm -r ";
	cmd += f;
	cmd += " 2>/dev/null";
	//cerr << "COUCOU " << cmd << '\n';
	system(cmd.c_str());
#endif
}

/** 
 * @brief Create some files in 3 subdirectories of the input directory
 * 
 */
BdbhTest::BdbhTest(const string& n): input_dir(n) {

	// In a real application, should be done at start
	bdbh::Initialize();

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
	
	// Nothing to create if inputdir already exists !
	struct stat buf;
	int err = stat(getInputDir().c_str(), &buf);
	if (err==-1 && errno==ENOENT) {
	
		// Create the hierarchy
		mkdir(getInputDir().c_str(),0700);
		string dir1=getInputDir() + "/C";
		string dir2=getInputDir() + "/C/C";
		string dir3=getInputDir() + "/D";
		mkdir(dir1.c_str(),0700);
		mkdir(dir2.c_str(),0700);
		mkdir(dir3.c_str(),0700);
	
		// Create the files
		for (size_t i=0;i<files.size();++i) {
			createFile(getInputDir(),files[i]);
		}
	}
	else if (err == -1) {
		throw(runtime_error("ERROR"));
	};
}

BdbhTest::~BdbhTest() { 
	string d1 = getDbDir();
	string d2 = INPUTDIR1;
	d2 += ".extracted*";
	string d3 = d1 + ".tomerge*";
	removeFile(d1);
	removeFile(d2);
	removeFile(d3);
	
	// In a real application, should be done at end
	bdbh::Terminate();
};

