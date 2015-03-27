
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
    Parameters(int, char**) throw (runtime_error);

	string getInDir()     const { return input_directory; };
	string getOutDir()    const { return output_directory; };
	string getTmpDir()    const { return tmp_directory; };
	string getInFile()    const { return input_file; };
	string getFileType()  const { return file_type; };
	string getCmdIsMpi()  const { return cmd_is_mpi; };
	bool isBdBh()         const { return is_bdbh; };
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
	void check();
	void checkEmptyMembers();
	void checkInputDirectory();
	//void checkOutputDirectory();
	void checkBlockSize();

	string getLastErrorText(const CSimpleOpt& arg);
	void usage();

	string input_directory;
	string output_directory;
	string tmp_directory;
	string input_file;
	string file_type;
	string cmd_is_mpi;
	bool is_bdbh;
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
