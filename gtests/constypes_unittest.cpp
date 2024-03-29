/** This file is part of chdb software
 * 
 * chdb helps users to run embarrassingly parallel jobs on a supercomputer
 *
 * chdb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2015-2018    C A L M I P
 *  chdb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with chdb.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

#include <errno.h>
#include "constypes_unittest.hpp"
#include "../src/system.hpp"
#include <fstream>
using namespace std;

/** 
 * @brief Create a File and write the content inside it
 * 
 * @param[in]  d         Path to the directory containing the file
 * @param[in] n         Struct: name and content
 */
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
    cerr << f << "\n";
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

// REMOVED string readFileFromBdbh(const string& db, const string& key) {
// REMOVED     string cmd = "./bdbh --database ";
// REMOVED     string tmp_out = "tmp.out";
// REMOVED     cmd += db;
// REMOVED     cmd += " cat ";
// REMOVED     cmd += key;
// REMOVED     cmd += " >";
// REMOVED     cmd += tmp_out;
// REMOVED     cerr << cmd << "\n";
// REMOVED     system(cmd.c_str());
// REMOVED     string out = readFile(tmp_out);
// REMOVED #ifdef REMOVE_OUTPUT
// REMOVED     unlink(tmp_out.c_str());
// REMOVED #endif
// REMOVED     return out;
// REMOVED }

bool existsFile(const string &f) {
    ifstream in(f.c_str());
    return (bool) in;
}

// REMOVED bool existsFileFromBdbh(const string& db, const string& key) {
// REMOVED     string cmd = "./bdbh --database ";
// REMOVED     cmd += db;
// REMOVED     cmd += " ls ";
// REMOVED     cmd += key;
// REMOVED     int rvl = system(cmd.c_str());
// REMOVED     return (rvl==0);
// REMOVED }
   
void removeFile(const string& f) {
#ifdef REMOVE_OUTPUT
    string cmd = "rm -r ";
    cmd += f;
    cmd += " 2>/dev/null";
    callSystem(cmd,false);
#endif
}

/** 
 * @brief Create 5 files in 3 subdirectories of the input directory, one of them produces an error
 * 
 */
ChdbTest1::ChdbTest1(): ChdbTest(INPUTDIR1) {

    // Nothing to do if inputdir already exists !
    struct stat buf;
    int err = stat(getInputDir().c_str(), &buf);
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

    expected_file_contents_with_rank["B.txt"]     = "RANK\t1\nSIZE\t2\nSTS\t0\nTXT\tABCDEFGHIJKLMNO\n\n";
    expected_file_contents_with_rank["C/C.txt"]   = "RANK\t1\nSIZE\t2\nSTS\t0\nTXT\tABCDEFGHIJKLMNOPQ\n\n";
    expected_file_contents_with_rank["C/C/C.txt"] = "RANK\t1\nSIZE\t2\nSTS\t0\nTXT\tC\n\n";
    expected_file_contents_with_rank["D/C.txt"]   = "RANK\t1\nSIZE\t2\nSTS\t1\nTXT\tABC\n\n";
    expected_file_contents_with_rank["A.txt"]     = "RANK\t1\nSIZE\t2\nSTS\t0\nTXT\tABCDEF\n\n";
}

/** 
 * @brief Create 10 files in the input directory, file 0.txt produces an error
 * 
 */
ChdbTest2::ChdbTest2(): ChdbTest(INPUTDIR2) {

    // Nothing to do if inputdir already exists !
    struct stat buf;
    int err = stat(getInputDir().c_str(), &buf);
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
        mkdir(getInputDir().c_str(),0700);
    
        // Create the files
        for (size_t i=0;i<files.size();++i) {
            createFile(getInputDir(),files[i]);
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

/** 
 * @brief Create 10 files in the input directory, file 9.txt produces an error
 * 
 */
ChdbTest3::ChdbTest3(): ChdbTest(INPUTDIR3) {

    // Nothing to do if inputdir already exists !
    struct stat buf;
    int err = stat(getInputDir().c_str(), &buf);
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
        mkdir(getInputDir().c_str(),0700);
    
        // Create the files
        for (size_t i=0;i<files.size();++i) {
            createFile(getInputDir(),files[i]);
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

/** 
 * @brief Create 5 .dir directories in the input directory
 * 
 */
ChdbTest4::ChdbTest4(): ChdbTest(INPUTDIR4) {

    // Nothing to do if inputdir already exists !
    struct stat buf;
    int err = stat(getInputDir().c_str(), &buf);
    if (err==-1 && errno==ENOENT) {
    
        // Create and populate expected_input_files
        
        // Create 5 directories in the directories, the last will generate an error (see ext_cmd.sh)
        // these files will be considered by Directories
        expected_input_files.push_back("0.dir");
        expected_input_files.push_back("0.dir/1.dir");
        expected_input_files.push_back("C/2.dir");
        expected_input_files.push_back("3.dir");
        expected_input_files.push_back("4.dir");
        
        // Create the hierarchy
        mkdir(getInputDir().c_str(),0700);
        string dir1=getInputDir() + "/C";
        mkdir(dir1.c_str(),0700);
    
        // Create the .dir directories
        for (size_t i=0;i<expected_input_files.size();++i) {
            string d = getInputDir() + '/' + expected_input_files[i];
            mkdir(d.c_str(),0700);
        }
    }
    else if (err == -1) {
        throw(runtime_error("ERROR"));
    };

    // Expected created files in those directories
    expected_file_pathes.push_back("out.txt");
    expected_file_pathes.push_back("out.txt");
    expected_file_pathes.push_back("out.txt");
    expected_file_pathes.push_back("out.txt");
    expected_file_pathes.push_back("out.txt");
}

// REMOVED void ChdbTestsWithParamsUsingBdbh::cvtInputDir(const string& src) {
// REMOVED     string dst = cmplInputDir(src);
// REMOVED     struct stat buf;
// REMOVED     int err = stat(dst.c_str(), &buf);
// REMOVED     if (err==-1 && errno==ENOENT) {
// REMOVED //        string cmd="cp -a ";
// REMOVED //        cmd += src;
// REMOVED //        cmd += " ";
// REMOVED //        cmd += dst;
// REMOVED         string cmd_head = "../bdbh/bdbh --database ";
// REMOVED         cmd_head += dst;
// REMOVED         string cmd = cmd_head;
// REMOVED         cmd += " create --compress";
// REMOVED         callSystem(cmd,true);
// REMOVED            cmd = cmd_head;
// REMOVED         cmd += " add -r ";
// REMOVED         cmd += src;
// REMOVED         callSystem(cmd,true);
// REMOVED     }
// REMOVED }
