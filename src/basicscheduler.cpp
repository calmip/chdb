/**
 * @file   basicscheduler.cpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Tue Sep 30 09:57:49 2014
 * 
 * @brief  
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

#include <mpi.h>
#include <iostream>
using namespace std;

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <algorithm>
#include <cmath>
#include <cerrno>

#include "system.hpp"
#include "basicscheduler.hpp"
#include <sys/types.h>
#include <dirent.h>

// The tags MPI defined by CHDB
#define CHDB_TAG_READY 1000
#define CHDB_TAG_GO    1010
#define CHDB_TAG_END   1020

void BasicScheduler::mainLoop() {

// May be useful for debugging with gdb
/*    
    {
    pid_t pid = getpid();
    ostringstream s_tmp;
    s_tmp << "gdbserver host:2345 --attach " << pid << "&";
    cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
    system(s_tmp.str().c_str());
    sleep(5);
    }
*/

    // some initialization specific to the master
    if (isMaster()) {
        mainLoopProlog();
    }

    // A barrier to wait for slave initialization
    MPI_Barrier(MPI_COMM_WORLD);

    // Enter the loop
    if (isMaster()) {
        mainLoopMaster();
    } else {
        mainLoopSlave();
    }
}
/***
 * @brief Called ONLY on Master BEFORE calling MainLoop, to initialize several stuff
 * 
 * @param err_file
 * @param report_file
 * 
 ****/

void BasicScheduler::mainLoopProlog() {

    // read the file names
    dir.readFiles();
    
    // Init the list of files to treat
    _initCheckList();

    // check the number of blocks versus number of slaves and throw an exception if too many slaves
    size_t slaves_max = dir.getNbOfFiles()/prms.getBlockSize();
    if (dir.getNbOfFiles() % prms.getBlockSize() != 0) {
        slaves_max += 1;
    };

    if (slaves_max<getNbOfSlaves()) {
        ostringstream out;
        out << "ERROR - You should NOT use more than " << slaves_max << " slaves";
        throw(runtime_error(out.str()));
    }

    // Create the output directory unless not specified (possible with 'dir' file type)
    if (prms.getOutDir() != "") {
        dir.makeOutDir(false,false);
    }

    // Open the error file, if any
    // NOTE - opened here and not in the constructor because it should NOT be opened on slaves !
    openErrFileIfNecessary();

    // Open the report file, if any
    // NOTE - opened here and not in the constructor because it should NOT be opened on slaves !
    openReportFileIfNecessary();

    return_values.clear();
    wall_times.clear();
}

/*******
 * \brief Open the err_file if prms is OK
 * 
 ******************************************/
void BasicScheduler::openErrFileIfNecessary() {
    if (!prms.isAbrtOnErr()) {
        string err_name = prms.getErrFile();
        err_file.open(err_name.c_str());
        if (!err_file.good()) {
            string msg = "ERROR - File could not be opened: ";
            msg += err_name;
            throw(runtime_error(msg));
        }
    }
}

/*******
 * \brief Open the report_file if prms is OK
 * 
 ******************************************/
void BasicScheduler::openReportFileIfNecessary() {
    if (prms.isReportMode()) {
        string report_name = prms.getReport();
        report_file.open(report_name.c_str());
        if (!report_file.good()) {
            string msg = "ERROR - File could not be opened: ";
            msg += report_name;
            throw(runtime_error(msg));
        } else {
            reportHeader(report_file);
        }
    }
}




    
/** 
 * @brief Send to the slave a message with no data and END TAG
 * 
 * @param slave 
 */

void BasicScheduler::sendEndMsg(void* send_bfr, int slave) {
    // Send an END message to the slave
    MPI_Send(send_bfr, 0, MPI_BYTE, slave, CHDB_TAG_END, MPI_COMM_WORLD);
}

void BasicScheduler::masterWaitForSlaves(void* recv_bfr, size_t bfr_size, int tag, MPI_Status& sts) {
    MPI_Request request;
    int flag = 0;
    MPI_Irecv((char*)recv_bfr,(int)bfr_size, MPI_BYTE, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &request);
    
    do {
        MPI_Test(&request,&flag,&sts);
        sleepMs(SLEEP_TIME);
    } while(flag==0);
}

/** 
 * @brief The main loop for the master
 * 
 */
void BasicScheduler::mainLoopMaster() {
    MPI_Status sts;

    // loop over the file blocks
    // Listen to the slaves
    size_t bfr_size=0;
    void* send_bfr=NULL;
    void* recv_bfr=NULL;

    allocBfr(send_bfr,bfr_size);
    allocBfr(recv_bfr,bfr_size);

    file_pathes = dir.nextBlock();

    while(!file_pathes.empty()) {

        // Prepare the send buffer for the next message
        size_t send_msg_len=0;
        writeToSndBfr(send_bfr,bfr_size,send_msg_len);
        
        // Listen to the slaves
        masterWaitForSlaves(recv_bfr, bfr_size, CHDB_TAG_READY, sts);
        //MPI_Recv((char*)recv_bfr,(int)bfr_size, MPI_BYTE, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
    //string received((const char*)recv_bfr,80);
    //cerr << received << '\n';

        int talking_slave = sts.MPI_SOURCE;

        // Init return_values, may be wall_times, and file_pathes with the message
        readFrmRecvBfr(recv_bfr);
        
        // counting the files
        treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);

        // checking the list items - If using bdbh, the temporary databases should be already synced
        _checkListItems(file_pathes,return_values);
        
        // Write info to report
        if (prms.isReportMode()) {
            reportBody(report_file, talking_slave);
        }

        // Handle the error, if error found and abort mode exit from the loop
        bool err_found = errorHandle();
        if (err_found && prms.isAbrtOnErr()) {
            sendEndMsg(send_bfr, talking_slave);
            break;

        }

        // Send the block to the slave
        MPI_Send(send_bfr,send_msg_len,MPI_BYTE,talking_slave,CHDB_TAG_GO,MPI_COMM_WORLD);

        // Init return_values and file_pathes for next iteration
        return_values.clear();
        wall_times.clear();
        file_pathes = dir.nextBlock();
    }

    // No more files to compute, but there are still some slaves working
    // loop over the slaves: when each slave is ready, send him a msg END
    //                       and when the slave sends back a tag END, consolidate his work and forget him
    //
    int working_slaves = getNbOfSlaves(); // The master is not a slave
    while(working_slaves>0) {
        masterWaitForSlaves(recv_bfr, bfr_size, MPI_ANY_TAG, sts);
        //MPI_Recv(recv_bfr, bfr_size, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &sts);
//    string received((const char*)recv_bfr,80);
//    cerr << received << '\n';
        int talking_slave = sts.MPI_SOURCE;
        int tag = sts.MPI_TAG;

        // Init return_values and file_pathes with the message
        readFrmRecvBfr(recv_bfr);
        
        // We received a tag "READY": send the message END
        if (tag==CHDB_TAG_READY) {

            // counting the files
            treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);
            
            // Handle the error
            errorHandle();
        
            // Write info to report
            if (prms.isReportMode()) {
                reportBody(report_file, talking_slave);
            }

            // Send an empty message tagged END to the slave
            sendEndMsg(send_bfr, talking_slave);
        }

        // We received a tag "END": consolidate data (if necessary) and forget this slave
        // We consolidate from 
        else {
            dir.consolidateOutput(false,file_pathes[0]);
            working_slaves--;
        }
    }

    // close err_file
    if (err_file.is_open()) err_file.close();

    // write report summary
    if (prms.isReportMode()) {
        reportSummary(report_file);
    }

    // free memory
    free(send_bfr);
    free(recv_bfr);
}

/** 
 * @brief Write the header in the report file
 *        Assign the vector wall_time_slaves
 * 
 * @param os 
 */
void BasicScheduler::reportHeader(ostream& os) {
    os << "SLAVE\tTIME(s)\tSTATUS\tINPUT PATH\n";

    wall_time_slaves.clear();
    wall_time_slaves.assign(getNbOfSlaves()+1,0.0);
    files_slaves.assign(getNbOfSlaves()+1,0);
}

/** 
 * @brief Write some report lines (called by MainLoopMaster)
 *        Update the corresponding cell of wall_time_slaves
 * 
 * @pre wall_times, file_pathes, return_values correctly initialized
 * @param os 
 * @param rank 
 */
void BasicScheduler::reportBody(ostream& os, int rank) {
    double wall_time_slave=0;
    int n=0;
    for (size_t i=0; i<file_pathes.size(); ++i) {
        if (file_pathes[i].size()!=0) {
            os << rank << '\t';
            os << wall_times[i] << '\t';
            os << return_values[i] << '\t';
            os << file_pathes[i] << '\n';
            wall_time_slave += wall_times[i];
            n += 1;
        }
    }
    wall_time_slaves[rank] += wall_time_slave;
    files_slaves[rank]     += n;
}

/** 
 * @brief Write the wall_time of each slave
 * 
 * @param os 
 */
void BasicScheduler::reportSummary(ostream& os) {
    os << "----------------------------------------------\n";
    os << "SLAVE\tN INP\tCUMULATED TIME (s)\n";
    size_t nb_slaves = getNbOfSlaves();
    double min=1E6;
    double max=0;
    double avg=0;
    double std=0;
    for (size_t i=1; i<=nb_slaves; ++i) {
        if (wall_time_slaves[i]<min) min=wall_time_slaves[i];
        if (wall_time_slaves[i]>max) max=wall_time_slaves[i];
        avg += wall_time_slaves[i];
        os << i << '\t' << files_slaves[i] << '\t' << wall_time_slaves[i] << '\n';
    }

    // computing the average time and standard deviation
    avg /= nb_slaves;
    std =  0;
    if (nb_slaves>1) {
        for (size_t i=1;i<=nb_slaves; ++i) {
            std += (wall_time_slaves[i]-avg)*(wall_time_slaves[i]-avg);
        }
        std = sqrt(std/(nb_slaves-1));
    }

    os << "----------------------------------------------\n";
    os << "AVERAGE TIME (s)        = " << avg << '\n';
    os << "STANDARD DEVIATION (s)  = " << std << '\n';
    os << "MIN VALUE (s)           = " << min << '\n';
    os << "MAX VALUE (s)           = " << max << '\n';
}
    
/** 
 * @brief The main loop for the slaves
 * 
 */
void BasicScheduler::mainLoopSlave() {
    vector_of_strings blk;
    MPI_Status sts;

    size_t bfr_size=0;
    void* bfr=NULL;
    allocBfr(bfr,bfr_size);
/*
    {
        pid_t pid = getpid();
        ostringstream s_tmp;
        s_tmp << "gdbserver host:2345 --attach " << pid << "&";
        cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
        system(s_tmp.str().c_str());
        sleep(5);
    }
*/

    // Sleep a while before starting, this may be good for the I/O subsystem
    // Every slave sleeps sleep_time * rank, thus the highest the rank the more you sleep
    // We sleep ONLY at the beginning
    unsigned int sleep_time = prms.getSleepTime();
    if (sleep_time != 0) {
        sleep(sleep_time*rank);
    }
    
    // all msgs are sent/received to/from the master
    const int master    = 0;
    int tag             = CHDB_TAG_GO;
    size_t send_msg_len = 0;
    while(tag==CHDB_TAG_GO) {

        // Prepare the send buffer for the next message
        writeToSndBfr(bfr,bfr_size,send_msg_len);

        // Send the report+ready message to the master, receive a list of files to treat
        MPI_Sendrecv_replace((char*)bfr,(int)bfr_size,MPI_BYTE,master,CHDB_TAG_READY,master,MPI_ANY_TAG,MPI_COMM_WORLD,&sts);
        tag = sts.MPI_TAG;

        // Init file_pathes with the message
        readFrmRecvBfr(bfr);

        if (tag==CHDB_TAG_GO) {
            executeCommand();
        }
    }

    // END tag received: consolidate output directory from temporary and leave
    // If first_execution:
    //    - The slave did nothing !
    //    - The temporary directory is NOT initialized
    //    ---> There is nothing to consolidate, and calling consolidateOutput will throw an exception
    if (!first_execution) {
        dir.consolidateOutput(true);
    }

    // Send a last message to the master: tag END, name of consolidated output directory
    file_pathes.clear();
    return_values.clear();
    wall_times.clear();
    if (!first_execution) {
        file_pathes.push_back(dir.getOutDir());
    }

    writeToSndBfr(bfr,bfr_size,send_msg_len);
    MPI_Send(bfr, bfr_size, MPI_BYTE, master, CHDB_TAG_END, MPI_COMM_WORLD);

    // free memory
    free(bfr);
}

/** 
 * @brief Execute the command on all files of file_pathes, store the result in return_values and may be the wall time in wall_times
 *        This function should be called by the slaves only
 * 
 */
void BasicScheduler::executeCommand() {

    // Create the output and temporary directories when first-time call
    // If the output directory already exists, it will be removed
    // NOTE - The creation of those directories is defferred as much as possible,
    // so that the master can execute an MPI_Abort during initialization, without 
    // letting temporaries on the disk !
    if (first_execution) {
        dir.makeOutDir(true,true);
        dir.makeTempOutDir();
        first_execution = false;
    }

    string command = prms.getExternalCommand();
    int zero = 0;
    double zerod = 0.0;
    return_values.assign (file_pathes.size(),zero);

    if (prms.isReportMode()) {
        wall_times.assign(file_pathes.size(),zerod);
    }

    for (size_t i=0; i<file_pathes.size(); ++i) {
        string cmd = command;
        string in_path = file_pathes[i];

        // skip input files with empty names
        if (in_path.size()==0) continue;
        dir.completeFilePath(in_path,cmd);

        // If dir file type, DO NOT prepent outdirectory to the output file path
        // because in this case the output file should be computed from the current (work) directory
        bool outdir_complete = !prms.isTypeDir();

        vector_of_strings out_files = prms.getOutFiles();
        for (size_t j=0; j<out_files.size(); ++j) {
            dir.completeFilePath(in_path,out_files[j],outdir_complete);
        }

        string work_dir = prms.getWorkDir();
        if (work_dir.length() != 0) {
            // Give a real name to work_dir, and force to start with output directory name
            dir.completeFilePath(in_path,work_dir,outdir_complete);
        }

        string snippet = prms.getEnvSnippet();
        if (work_dir.length() != 0) {
            // Complete file paths in snippet, too
            dir.completeFilePath(in_path,snippet);
        }

        double start = MPI_Wtime();

        // We have only one input file yet, but this could change in the future
        vector_of_strings in_pathes;
        in_pathes.push_back(in_path);
        int sts = dir.executeExternalCommand(in_pathes,cmd,out_files,work_dir,snippet);
        double end = MPI_Wtime();

        // If abort on Error, throw an exception if status != 0
        if (sts!=0) {
            //    if (prms.isAbrtOnErr()) {
            //    ostringstream msg;
            //    msg << "ERROR with external command\n";
            //    msg << "command    : " << cmd << '\n';
            //    msg << "exit status: " << sts << '\n';
            //    msg << "ABORTING - Try again with --on-error\n";
            //    throw(runtime_error(msg.str()));
            //} else {
            return_values[i] = sts;
            //}
        }

        // Store the time elapsed if report mode
        if ( prms.isReportMode() ) {
            wall_times[i] = end - start;
        }
    }
    
    // Sync the temporary and output databases, for security
    // If a signal is received after this call, the files are stored in the databases
    dir.Sync();
}

/** 
 * @brief Called by the MainLoopMaster when error mode is on, writes to err_file
 *        the errors found in the last treated block and return true if some error is found
 * 
 * @pre return_values is filled with the returned values
 * @pre file_pathes is filled with the treated file pathes, in the same order as return_values
 * 
 * @return true = error found, false = no error
 */
bool BasicScheduler::errorHandle() {

    // find the first value in error and return false if none
    vector_of_int::iterator it = find_if(return_values.begin(),return_values.end(),isNotNull);
    if (it == return_values.end()) {
        return false;

    } else {

        // If not Abort On Error, loop from it and write the files in error
        if (!prms.isAbrtOnErr()) {
            for ( size_t i=it-return_values.begin(); i<return_values.size(); ++i) {
                if (return_values[i]==0) continue;
                err_file << *it << '\t' << file_pathes[i] << '\n';
            }

            // If Abort on Error, write a message to cerr before aborting !
        } else {
            cerr << "ERROR with external command - sts=";
            cerr << *it << " ";
            cerr << "input file=" << file_pathes[it-return_values.begin()] << '\n';
            cerr << "ABORTING - Try again with --on-error\n";
        }

        // Error found !
        return true;
    }
}

/** 
 * @brief Alloc a buffer big enough to send/receive return_values and file_pathes
 *        For block_size 4 and FILEPATH_MAXLENGTH 5 we may have at most:
 *          4000AAAABBBBCCCCDDDD4000xxxxx\0xxxxx\0xxxxx\0xxxxx\0
 *          4000 is the integer representation of 4 in little endian machines
 *          A,B,C,D are integers representing the status retrieved by the slaves for each file name
 *
 * @param[out] bfr The allocated buffer
 * @param[out] bfr_sze The size of the allocated buffer
 *
 */
void BasicScheduler::allocBfr(void*& bfr,size_t& bfr_size) {
    size_t vct_size = prms.getBlockSize();
    bfr_size  = sizeof(int) + vct_size*sizeof(int);
    bfr_size += sizeof(int) + vct_size*sizeof(double);
    bfr_size += sizeof(int) + vct_size*(FILEPATH_MAXLENGTH+1);
    bfr       = malloc(bfr_size);
    if (bfr==NULL) {
        throw(runtime_error("ERROR - Could not allocate memory"));
    }
}

/** 
 * @brief Write to a send buffer the vectors return_values, may be wall_times, and file_pathes
 * 
 * @pre The bfr should be already allocated with allocBfr

 * @param[in]  bfr       The buffer (should be already allocated)
 * @param[in]  bfr_size  The buffer length
 * @param[out] data_size The data stored, should be <= bfr_size
 * 
 */
void BasicScheduler::writeToSndBfr(void* bfr, size_t bfr_size, size_t& data_size) {
    checkInvariant();
    size_t int_data_size=0;
    size_t str_data_size=0;
    size_t dbl_data_size=0;

    // Mode No Report:
    // ---------------
    //
    // Fill the buffer with the data from return_values, then from file_pathes
    //      iiiiiiiiifffffffffffffffffffffffffff00000000000000000000000000000
    //      ^        ^                          ^
    //      0        int_data_size              int_data_size+str_data_size  ^bfr_size
    //
    // Mode --report:
    // --------------
    //
    // Fill the buffer with the data from return_values, from wall_times, then from file_pathes
    //     iiiiiiiiidddddddddfffffffffffffffffffffffffff00000000000000000000000000000
    //     ^        ^        ^-----v                    ^-------v                   ^----------------------v
    //     0        int_data_size  int_data_size+dbl_data_size  int_data_size+dbl_data_size+str_data_size  bfr_size
    
    vctToBfr(return_values,bfr,bfr_size,int_data_size);
    bfr      = (void*) ((char*) bfr + int_data_size);
    bfr_size = bfr_size - int_data_size;
    data_size= int_data_size;
    if (prms.isReportMode()) {
        vctToBfr(wall_times,bfr,bfr_size,dbl_data_size);
        bfr = (void*)((char*) bfr + dbl_data_size);
        bfr_size = bfr_size - dbl_data_size;
        data_size += dbl_data_size;
    }
    vctToBfr(file_pathes,bfr,bfr_size,str_data_size);
    data_size += str_data_size;
}

/** 
 * @brief Read a receive buffer and initialize the vectors return_values, may be wall_times, and file_pathes
 * 
 * @param bfr 
 */
void BasicScheduler::readFrmRecvBfr    (const void* bfr) {
    size_t data_size;
    bfrToVct(bfr,data_size,return_values);
    bfr = (void*) ((char*) bfr + data_size);
    if (prms.isReportMode()) {
        bfrToVct(bfr,data_size,wall_times);
        bfr = (void*) ((char*) bfr + data_size);
    }
    bfrToVct(bfr, data_size, file_pathes);
    checkInvariant();
}

/** 
 * @brief The invariants:
 *              1/ return_values empty, OR same size as file_pathes
 *              2/ wall_times empty, OR same size as file_pathes
 *
 */
void BasicScheduler::checkInvariant() {
    assert(return_values.empty() || return_values.size()==file_pathes.size());
    assert(wall_times.empty() || wall_times.size()==file_pathes.size());
}
    
