/**
 * @file   directories.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Mon Sep 29 11:03:01 2014
 * 
 * @brief  This class manages the files contained inside the input or output directory
 *         It has several subclasses, for managing the different types of directories 
 *         (real directory, bdbh file, etc)
 * 
 * This file is part of chdb software
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
 * 
 */

#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include <iostream>
#include <list>
#include <algorithm>
#include <set>
using namespace std;

#include "constypes.hpp"
#include "parameters.hpp"

// The size max of a file path
#define FILEPATH_MAXLENGTH 2000

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
    Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};

class Directories: private NonCopyable {

public:
    Directories(const Parameters& p):prms(p),rank(-1),comm_size(0),blk_ptr(files.begin()) {};
    virtual ~Directories(){};
    
    // setRank should be set ONLY ONE TIME (this is checked)
    // ONLY IF master, setRank calls checkParameters and throws a runtime_exception if there is something wrong
    void setRank(int,int);

    virtual void   makeOutDir(bool,bool) = 0;
    virtual void   makeTempOutDir()  = 0;
    virtual string getOutDir() const = 0;
    virtual string getTempOutDir() const = 0;
    virtual string getTempInDir() const = 0;
    virtual void findOrCreateDir(const string &) = 0;
    virtual void buildBlocks(list<Finfo>&, vector_of_strings&) const;

    // consolidateOutput: the slaves may write output to temporaries (this depends on children of Directories)
    //                    All those temporaries are consolidated at the end
    virtual void consolidateOutput(bool from_temp, const string& path="") = 0;

    // Explaining how to consolidate data manually !
    virtual string howToConsolidate() const = 0;

    const vector_of_strings& getFiles() {
        readFiles();
        return files;
    }
    
    // If in iter mode, generate the list of files
    // Else, delegate this to v_readFiles, a virtual pure.
    void readFiles();

    vector_of_strings nextBlock();
    void completeFilePath(const string& p, string& text, bool force_out=false);
    // input files, cmd, output files
    virtual int executeExternalCommand(const vector_of_strings&,const string&,const vector_of_strings&,const string& wd="",const string& sn="") = 0;
    size_t getNbOfFiles() { readFiles(); return files_size; };

    friend class TestCase1_block1_Test;
    friend class TestCase1_usingFsfindOrCreateDir_Test;

    // Derived classes may override those functions if they have something to close in emergency...
    virtual void Sync() {};
    virtual void SetSignal(int signal) {
        //cerr << "Directory rank="<< rank <<" received a signal - " << signal << " - Ignoring it !" << endl;
    };
    
protected:
    void initInputFiles() const;
    bool isCorrectType(const string &, bool) const;

    const Parameters& prms;
    
    // The input files to be treated
    mutable vector_of_strings files;
    mutable size_t files_size;
    
    // The files which are specified through the switch --in-files
    mutable set<string> input_files;
    int rank;
    int comm_size;
    void buildMpiCommand(string&) const;

private:
    mutable vector_of_strings::iterator blk_ptr;
//    void replaceTmpl(const string& tmpl, const string& value, string& text);
    virtual void v_readFiles() = 0;
    // Check the parameters, they should be coherent, but this depends on the derived classes
    virtual void checkParameters() = 0;

};

#endif
