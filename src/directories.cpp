/**
 * @file   directories.cpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
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
 *  Copyright (C) 2015-2022    C A L M I P
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

#include <iostream>
#include <fstream>
using namespace std;

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <list>
#include <cerrno>

#include "usingfs.hpp"
#include <sys/types.h>
#include <dirent.h>
#include "system.hpp"

int operator<(const Finfo& a, const Finfo& b) { return a.st_size < b.st_size; }

/** 
 * @brief set the protected member rank and comm_size, 
 *        if rank=0 call check_parameters
 * 
 * @param r 
 * @param s 
 *
 * @exception the rank and comm_size can be set only 1 time
 */

void Directories::setRank(int r, int s) {
    if (rank>=0) {
        throw(logic_error("ERROR - rank already set"));
    }
    rank=r;
    comm_size=s;
    // We check the parameters only for the master process
    // If problem detected throw runtime_error
    if (rank==0) checkParameters();
}

/**
 *  @brief Return the nextblok of file names
 *
 *  @pre The vector files should have been initialized
 *  @return A vector of names
 *          An empty vector if there are no more file names
*/

vector_of_strings Directories::nextBlock() {
    vector_of_strings blk;
    if ( blk_ptr != files.end() ) {

        int blk_size = prms.getBlockSize();
        vector_of_strings::iterator blk_next_ptr;
        if (files.end()-blk_ptr > blk_size) {
            blk_next_ptr = blk_ptr + blk_size;
        } else {
            blk_next_ptr = files.end();
        }
        blk.assign(blk_ptr,blk_next_ptr);
        blk_ptr = blk_next_ptr;
    }
    return blk;
}
/** 
 * @brief Replace some templates in the parameter, using the file_path:
 *        file pathes: #path#, #dirname#
 * 
 * @param[in]  p         Path used as a source for the template expansion
 * @param[out] text      String to expand (generally a command line or a file name)
 * @param[in]  force_out If true, output directory is forced at begininning of text
 */
void Directories::completeFilePath(const string& p, string& text, bool force_out) {

    // We must work with a readwrite copy !
    // p is the path (a/toto.txt), d the dirname (a), n the name (toto.txt), b the basename (toto), e the ext (txt)
    string d,n,b,e;
    parseFilePath(p,d,n,b,e);

    string id = getTempInDir();
    string od = getTempOutDir();
    static string tmpl1="%in-dir%";
    static string tmpl2="%out-dir%";
    static string tmpl3="%path%";
    static string tmpl4="%basename%";
    static string tmpl5="%name%";
    static string tmpl6="%dirname%";
    
    replaceTmpl(tmpl1,id,text);
    replaceTmpl(tmpl2,od,text);
    replaceTmpl(tmpl3,p,text);
    replaceTmpl(tmpl4,b,text);
    replaceTmpl(tmpl5,n,text);
    replaceTmpl(tmpl6,d,text);
    if (force_out) {
        size_t od_index=text.find(od);
        // if not found, or not found at start
        if (od_index!=0) {
            text = od + '/' + text;
        }
    }
}

/** 
 * @brief Should be called by the subclasses executeExternalCommand. Build an mpi command,
 *        using the env variable CHDB_MPI_CMD and the switch mpi-slaves
 * 
 * @param[inout]  cmd    Command to run, returned unchanged if no mpi, or completed
 *                       with the mpirun calling string if mpi in slave wanted
 */
void Directories::buildMpiCommand(string& cmd) const {
    string mpi_slaves = prms.getMpiSlaves();
    if ( mpi_slaves != "" ) { 
        const char * mpi_cmd_c = getenv("CHDB_MPI_CMD");
        if (mpi_cmd_c != NULL) {
            //cerr << "MOUMOUMOU #" << mpi_cmd_c << "#\n";
            string mpi_cmd = mpi_cmd_c;
            string h;
            getHostName(h);
            const char * mpi_c = getenv("CHDB_MPI");
            if (mpi_c == NULL)
            {
                replaceTmpl("%CHDB_MPI%", "", mpi_cmd);
            }
            else
            {
                string mpi = mpi_c;
                // If character other than a-z/0-9, do not replace (security)
                if (mpi.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789") == string::npos)
                {
                    replaceTmpl("%CHDB_MPI%", mpi, mpi_cmd);
                }
            }
            replaceTmpl("%MPI_SLAVES%", mpi_slaves, mpi_cmd);
            replaceTmpl("%HOSTNAME%", h, mpi_cmd);
            replaceTmpl("%COMMAND%", cmd, mpi_cmd);
            cmd = mpi_cmd;
        } else {
            throw runtime_error("ERROR -The env variable CHDB_MPI_CMD does not exists");
        }
    }
    //cerr << "COUCOUCOU #" << cmd << "#\n";

}

/** 
 * @brief Fill if possible the set of files to use
 * 
 */
void Directories::initInputFiles() const {
    string in_file = prms.getInFile();
    
    if (in_file != "") {
        ifstream in(in_file.c_str());
        if (!in.good()) {
            string msg = "ERROR - File could not be opened: ";
            msg += in_file;
            throw(runtime_error(msg));
        }

        // parse file: this should be a tsv file
        // lines starting by # are a comment and are ignored
        // empty lines are ignored
        // Lines with 2 fields and more are considered: (fields 2-)
        // Lines with 1 field are considered
        string tmp;
        while (in) {
            getline(in,tmp);
            if (tmp.size()!=0 && tmp[0]!='#') {
                size_t p=tmp.find_first_of('\t');
                if (p!=string::npos && p<tmp.length()-1) {
                    input_files.insert(tmp.substr(p+1));
                } else {
                    input_files.insert(tmp);
                }
            }
        }
    }
}

/******************
 * @brief Read the files to process
 *        If dir or file type, call v_readFiles
 *        If iter type, do the iterations
 * 
 ****************************/ 

void Directories::readFiles() {
    if (files.empty()) {
        if ( !prms.isTypeIter()) {
            v_readFiles();
            files_size = count_if(files.begin(), files.end(), isNotNullStr);
            if ( ! files.empty()) {
                blk_ptr=files.begin();
            }
        } else {
            // Init the list of input files if any
            initInputFiles();
            if (input_files.empty()) {
                for (unsigned int i = prms.getIterationStart(); i <= prms.getIterationEnd(); i += prms.getIterationStep() ) {
                    files.push_back(to_string(i));
                }
            } else {
                for (unsigned int i = prms.getIterationStart(); i <= prms.getIterationEnd(); i += prms.getIterationStep() ) {
                    string f=to_string(i);
                    if (input_files.find(f)!=input_files.end()) {
                        files.push_back(f);
                    }
                }
            }
            files_size = files.size();
            if ( ! files.empty() ) {
                blk_ptr = files.begin();
            }
        }
    }
}

/** 
 * @brief Check the name extension versus the required extension (file type)
 *        Check also the file type (Regular file or Directory)
 * 
 * @param name 
 * @param is_a_dir (true = directory)
 * 
 * @return true if the type is OK, false if not
 */
bool Directories::isCorrectType(const string & name,bool is_a_dir) const {
    string ext  = '.' + prms.getFileType();
    bool ext_ok = isEndingWith(name,ext);
    bool dir_ok = (is_a_dir == prms.isTypeDir());
    return ext_ok && dir_ok;
}

/** 
 * 
 * @brief Prepare the f vector for a balanced distribution of jobs (useful only if isSizeSort())
 *        1/ The f_info list is sorted from the longest to the shortest file
 *        2/ The file names are copied to f using an interleaved algorithm
 * 
 * @param f_info A list of Finfo objects (used to sort the files)
 * @param f      The final result
 *
 */

void Directories::buildBlocks(list<Finfo>& f_info,vector_of_strings&f) const {
    // sort files_tmp by sizes and copy the names to files
    f_info.sort();

    // Some parameters
    size_t nb_files   = f_info.size();
    size_t nb_slaves;
    if (comm_size>1) {
        nb_slaves = comm_size - 1;
    } else {
        throw(logic_error("ERROR - setRank has not been called"));
    }
    
    size_t block_size = prms.getBlockSize();
    size_t slice_size = nb_slaves*block_size;
    size_t dim_f = 0;
    if (nb_files%slice_size == 0) {
        dim_f = nb_files;
    } else {
        dim_f = slice_size * (1 + nb_files/slice_size);
    }

    // reserve enough size for f and init to ""
    f.clear();
    f.reserve(dim_f);
    f.assign(dim_f,(string)"");

    // Keep the file names to the vector files, reversing the order and interleaving them
    // Ex. 40 files, 4 slaves, block_size=5 see test usingFsSortFiles1
    // 1st slice = | 0  4  8 12 16 | 1  5  9 13 17|  2  6 10 14 18|  3  7 11 15 19|  4 blocks
    // 2nd slice = |20 24 28 32 36 |21 25 29 33 37| 22 26 30 34 38| 23 27 31 35 39|  4 blocks
    
    // Ex. 25 files, 4 slaves, block_size=5 see test usingFsSortFiles2
    // slice_size=20, 2 slices, dim_f=40
    // 1st slice = | 0  4  8 12 16 | 1  5  9 13 17|  2  6 10 14 18|  3  7 11 15 19|  4 blocks
    // 2nd slice = |20 24 "" "" "" |21 "" "" "" ""| 22 "" "" "" ""| 23 "" "" "" ""|  4 blocks (with holes)
    
    size_t i=0;
    for (list<Finfo>::reverse_iterator ri=f_info.rbegin();ri!=f_info.rend();++ri,++i) {
        size_t k = slice_size * (i / slice_size); // 0, 20
        k += block_size * (i % nb_slaves);        // k += 0, 5, 10, 15
        k += (i%slice_size) / nb_slaves;          // k += 0, 1, 2, 3, 4
        f[k] = ri->name;
    }
}

