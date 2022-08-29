/**
 * @file   parameters.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Mon Sep 29 11:03:01 2014
 * 
 * @brief  This class parses and keeps in memory the command line
 *
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


#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <vector>
#include <string>
#include <stdexcept>
using namespace std;


#include "SimpleOpt.h"
#include "constypes.hpp"

/** 
    \brief This class parses and keeps in memory the command line
*/

class Parameters: private NonCopyable {

public:
    // May throw a runtime_error if something wrong with the parameters
    Parameters(int, char**);

    string getInDir()     const { return input_directory; };
    string getOutDir(bool db_free=false)    const { return (!db_free) ? output_directory: output_directory_db_free; };
    string getWorkDir()   const { return work_directory; };
    unsigned int getSleepTime() const { return sleep_time;};
    string getEnvSnippet() const{ return env_snippet; };
    string getTmpDir()    const { return tmp_directory; };
    string getInFile()    const { return input_file; };
    unsigned int getIterationStart() const { if (isTypeIter()) return iter_start; else throw logic_error("ERROR - Not in TypeIter");};
    unsigned int getIterationEnd()   const { if (isTypeIter()) return iter_end; else throw logic_error("ERROR - Not in TypeIter");};
    unsigned int getIterationStep()  const { if (isTypeIter()) return iter_step; else throw logic_error("ERROR - Not in TypeIter");};
    string getFileType()  const { return file_type; };
    bool isTypeDir()      const { return is_type_dir; };
    bool isTypeIter()      const { return is_type_iter;};
    bool isBdBh()         const { return is_bdbh; };
    bool isTypeFile()     const { return is_type_file; };
    string getMpiSlaves() const { return mpi_slaves; };
    bool isInMemory()     const { return is_in_memory; };
    bool isSizeSort()     const { return is_size_sort; };
    bool isVerbose()      const { return is_verbose; };
    bool isAbrtOnErr()    const { return on_error.length()==0; };
    bool isTmpDir()       const { return tmp_directory.length()!=0; };
    string getErrFile()   const { return on_error; };
    bool isReportMode()   const { return report.length()!=0; };
    string getReport()    const { return report; };
    int getBlockSize()    const { return block_size; };
    vector_of_strings  getOutFiles() const { return output_files; };
    string getExternalCommand()      const { return external_command; };

private:
    void checkParameters();
    void checkEmptyMembers();
    void checkInputDirectory();
    //void checkOutputDirectory();
    void checkBlockSize();
    void setInputType(const string&);

    string getLastErrorText(const CSimpleOpt& arg);
    void usage();

    string input_directory;
    string output_directory;
    string output_directory_db_free;
    string work_directory;
    unsigned int sleep_time;
    string env_snippet;
    string tmp_directory;
    string input_file;
    string file_type;
    bool is_type_file;
    bool is_type_dir;
    bool is_type_iter;
    string mpi_slaves;
    unsigned int iter_start;
    unsigned int iter_end;
    unsigned int iter_step;
    bool is_bdbh;
    bool is_in_memory;
    bool is_size_sort;
    bool is_verbose;
    string on_error;
    string report;
    int block_size;
    vector_of_strings output_files;
    string external_command;
};

/** 
 * @brief This exception is thrown by Parameters when --help is called
 * 
 */
class ParametersHelp: public runtime_error {
public:
    ParametersHelp(): runtime_error("") {};
};

#endif
