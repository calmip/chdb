
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
	string getOutDir()    const { return output_directory; };
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
	bool isTypeIter()	  const { return is_type_iter;};
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
	void checkOutputDirectory();
	void checkBlockSize();
	void setInputType(const string&);

	string getLastErrorText(const CSimpleOpt& arg);
	void usage();

	string input_directory;
	string output_directory;
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

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/
