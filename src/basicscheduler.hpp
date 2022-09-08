/**
 * @file   basicscheduler.hpp
 * @author Emmanuel Courcelle <emmanuel.courcelle@inp-toulouse.fr>
 * @date   Tue Sep 30 09:52:07 2014
 * 
 * @brief  This class implements a basic scheduling strategy: individual jobs
 *         are launched one after another, as soon as a slave becomes available we send a new job.
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
 */

#ifndef BASIC_SCHEDULER_H
#define BASIC_SCHEDULER_H

using namespace std;

#include <mpi.h>
#include <memory>
#include "constypes.hpp"
#include "scheduler.hpp"

/**********************
 * The information sent by the slaves in Report mode
 **********************/
class BasicScheduler: public Scheduler {
public:
    BasicScheduler(const Parameters& p, Directories& d, bool o=false):Scheduler(p,d,o),first_execution(true),treated_files(0){};
    void mainLoop() override;
    bool errorHandle(const vector_of_int&,const vector_of_strings&) override;
    size_t getTreatedFiles() const override { return treated_files; };
 
//    friend class TestCase1_ExecuteCommandFrmList1_Test;
//    friend class TestCase1_ExecuteCommandFrmList2_Test;
    friend class ChdbTest1_readwriteToSndBfr_Test;
    friend class TestInvariant_ChdbTest;
//    friend class WithOrNoTmp_AbortOnError_Test;
//    friend class WithOrNoTmp_ContinueOnError_Test;
//    friend class TestCase1_AbortOnError_Test;
//    friend class TestCase1_ContinueOnError_Test;

private:
    vector_of_double wall_time_slaves;  // The cumulated elapsed time of each slave - maintained by the master
    vector_of_int files_slaves;         // Number of files treated by each slave ( maintained by the master
    vector_of_strings node_name_slaves; // The node name of each slave (only in Report mode)

    void mainLoopMaster();
    void mainLoopProlog();
    void mainLoopSlave();
    void masterWaitForSlaves(void* recv_bfr, size_t bfr_size, int tag, MPI_Status& sts);
    void sendEndMsg(void*, int);


    void executeCommand(const vector_of_strings&, vector_of_int&, vector_of_double&);
    bool first_execution;

    void checkInvariant(const vector_of_int&, const vector_of_double&, const vector_of_strings&);

    void allocBfr(unique_ptr<char[]>& bfr_mngr, size_t& bfr_sze);
    void writeToSndBfr (void*, size_t, size_t&,
                        const vector_of_int&, const vector_of_double&, const vector_of_strings&, const vector_of_strings&);
    void readFrmRecvBfr(const void*,
                        vector_of_int&, vector_of_double&, vector_of_strings&, vector_of_strings&);

    void reportHeader(ostream &);
    void reportBody(ostream&, int,
                    const vector_of_int&, const vector_of_double&, const vector_of_strings&, const vector_of_strings&);
    void reportSummary(ostream&);

    void openErrFileIfNecessary();
    void openReportFileIfNecessary();
    
    size_t treated_files;
};

#endif

