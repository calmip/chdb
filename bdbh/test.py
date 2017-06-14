#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import random
import glob
import time
from optparse import OptionParser

def _system(cmd,excep=True):
    '''Calling subprocess.Popen, raises an exception if error or returns stdout/stderr in an array'''
    #print "Execution de " + cmd
    p=subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    p.wait()
    if p.returncode !=0:
        if excep:
            raise BaseException("ERROR - cmd = " + cmd + " return value = " + str(p.returncode))
    else:
        #print "OK"
        pass
    return p.communicate()[0].split('\n')

def _find(top,dirs,files):
    '''some find emulation'''

    for f in glob.glob(top+'/*'):
        if os.path.isdir(f)==True:
            dirs.append(f)
            t=top+'/'+f
            _find(top+'/'+os.path.basename(f),dirs,files)
        else:
            files.append(f)
            
            
class TestResult(object):
    '''The result of the test are stored in this object'''
    infoFromDir  = []
    infoFromBdbh = []
    comparisons  = []
    time_add     = []
    time_xtract  = []

#
# @Params: 
#    options The parsed command line
#    n       The number of calls to __addFilesDir
#            
class Test(object):
    def __init__(self,options):
        "During initialization, we create the database, unless already done"

        self.dir               = options.dir
        self.dirperlevel       = options.dirperlevel
        self.filesperdirectory = options.filesperdirectory
        self.depth             = options.depth
        self.database = options.dir + ".bdbh"
        self.restit   = options.dir + ".restit"
        self.bdbh     = options.bdbh + " --database " + self.database + " "
        self.random   = options.random
        self.random1  = options.random + " 15 "
        self.time     = '/usr/bin/time --format="e=%e u=%U s=%S" '
        self.result   = TestResult()
        self.nb_dir_files     = self.__CompNbFiles()
        self.ttl_dir_files    = ([],[])
        self.nb_ttl_dir_files = (0,0)
        cmd = self.bdbh + "create"

        # Ignore the return code, it is != 0 if the databbase already exists
        _system(cmd,False)

        # Make the top directory unless already done
        if os.path.isdir(self.dir)==False:
            os.makedirs(self.dir)
        cmd = self.bdbh + "mkdir " + self.dir
        _system(cmd,False)

    def __CompNbFiles(self):
        '''Compute the number of directories and files created'''
        nb_dir = 0
        n      = self.dirperlevel
        f      = self.filesperdirectory
        n_pow  = 1
        for i in range(self.depth+1):
            nb_dir += n_pow
            n_pow  *= n
        return (nb_dir,nb_dir*f)

    def runTestCase(self,test_case):
        '''A test_case is a tuple on positive or negative int, call __runTest for each int'''

        for i in test_case:
            n = int(i)
            if (n>0):
                print "Adding "+str(n*self.nb_dir_files[0])+ " directories and "+str(n*self.nb_dir_files[1])+" files" 
            if (n<0):
                print "Removing "+str(-n*self.nb_dir_files[0])+ " directories and "+str(-n*self.nb_dir_files[1])+" files" 
            self.__runTest(n)

    def __runTest(self,n):
        '''Run a test: adding a hierarchy if n>0 or removing a lot of files+directories if n<0'''

        time_add = '0'
        for i in range(abs(n)) :

            if n>0:
                # Populate the top directory
                curdir = os.getcwdu()
                os.chdir(self.dir)
                self.__addFilesDir()
                os.chdir(curdir)
            
                # Add the hierarchy directory to the database
                cmd = self.time + self.bdbh + "add -r -o "
                cmd += self.dir
                time_add=_system(cmd)[0]
                
            if n<0:
                # Remove some files from the directory and the database
                self.__rmFilesDir()

        # Extract the database to an empty directory
        cmd = "rm -rf  " + self.restit
        _system(cmd)

        os.mkdir(self.restit)
        cmd = self.time + self.bdbh + "extract -r --directory " + self.restit
        cmd += " --root /" + self.dir
        time_xtract=_system(cmd,False)[0]
        
        # Compare top and restit directories
        cmd = "diff -rq " + self.dir + " " + self.restit
        comparison = _system(cmd,False)

        # Keep some data            
        self.ttl_dir_files    = self.__infoFromDir()
        self.nb_ttl_dir_files = (len(self.ttl_dir_files[0])+1,len(self.ttl_dir_files[1]))
        self.result.infoFromDir.append(self.nb_ttl_dir_files)
        self.result.infoFromBdbh.append(self.__infoFromBdbh())
        self.result.comparisons.append(comparison)
        self.result.time_add.append(time_add)
        self.result.time_xtract.append(time_xtract)


    def __rmFilesDir(self):
        '''Remove files and directories. Number in self.dir_files, randomly chosen'''

        nb_dirs   = self.nb_dir_files[0]
        if nb_dirs > len(self.ttl_dir_files[0]):
            nb_dirs = len(self.ttl_dir_files[0])

        nb_files = self.nb_dir_files[1]
        if nb_files > len(self.ttl_dir_files[1]):
            nb_files = len(self.ttl_dir_files[1])

        dirs = random.sample(self.ttl_dir_files[0],nb_dirs)
        files= random.sample(self.ttl_dir_files[1],nb_files)
#        dirs = random.sample(self.ttl_dir_files[0],5)
#        files= random.sample(self.ttl_dir_files[1],5)

        # Remove files, the directories
        # Remove from directory and from database simultaneously
        #print "removing " + str(len(files)) + "/" + str(self.nb_ttl_dir_files[1]) + " files"
        cmd_dir = "rm -f "
        cmd_bdbh = self.bdbh + "rm "
        for f in files:
            if f=='':
                continue
            _system(cmd_dir + f)
            _system(cmd_bdbh + f,False)

        #print "removing " + str(len(dirs)) + "/" + str(self.nb_ttl_dir_files[0]) + " directories"
        cmd_dir += "-r "
        cmd_bdbh+= "-r "
        for d in dirs:
            if d=='':
                continue
            #print "COUCOU " + d
            _system(cmd_dir + d)
            _system(cmd_bdbh + d,False)
            #print '\n'.join(_system(cmd_bdbh + d,False))

        
    def __addFilesDir(self,level=0):
        '''Create a lot of files and directories'''

        if level<self.depth:
            cmd = self.random1 + str(self.dirperlevel)
            subdirs = _system(cmd)[0].split(' ')
            #print subdirs
            curdir = os.getcwdu()
            for d in subdirs:
                #print "coucou "+d+" "+curdir
                if d=='':
                    continue
                # try to make a dir, if there is a file give up, if there is a dir no pb
                try:
                    os.mkdir(d)
                except:
                    if os.path.isfile(d)==True:
                        continue
                    
                os.chdir(d)
                cmd   = self.random1 + str(self.filesperdirectory)
                files = _system(cmd)[0].split(' ')
                for f in files:
                    if f=='':
                        continue
                    if os.path.isdir(f)==False:
                        cmd     = self.random + " 2000 3"
                        insight = _system(cmd)
                        fh      = open(f,'a')
                        for l in insight:
                            fh.write(l)
                            fh.write('\n')

                self.__addFilesDir(level+1)
                os.chdir(curdir)

    def __infoFromDir(self):
        '''return the list files and directories, reading the directory'''

        dirs = []
        files= []
        _find(self.dir,dirs,files)
        return (dirs,files)

    def __infoFromBdbh(self):
        '''return the number of files and directories, reading from the database'''
        cmd = self.bdbh + "info"
        info = _system(cmd)
        f  = int(info[3][32:])
        d  = int(info[4][32:])
        return (d,f)

def testCase(options):
    '''test case => Create a directory, and compare directory vs database'''

    print "YOUR TEST CASE = " + options.testcase
    test_case = [0]
    test_case += options.testcase.split(',')
    test = Test(options)
    test.runTestCase(test_case)

    compOk = ['']
    warningComp=[]
    fmt = "%15s%15s%6s%25s%25s"
    print fmt % ('info dir','info bdbh','Comp','time add','time xtr')
    for i in range(len(test_case)):
        c1 = str(test.result.infoFromDir[i])
        c2 = str(test.result.infoFromBdbh[i])
        c3 = ''
        if test.result.comparisons[i] != compOk:
            warningComp.append(False)
            c3 = 'KO'
        else:
            warningComp.append(True)
            c3 = 'OK'

        c4 = test.result.time_add[i].strip()
        c5 = test.result.time_xtract[i].strip()
        print fmt % (c1,c2,c3,c4,c5)

    if False in warningComp:
        for i in range(len(test_case)):
            if warningComp[i]==False:
                print "COMPARISON RESULT FOR %i" % i
                print '\n'.join(test.result.comparisons[i])
    
def main():
    parser = OptionParser(version="%prog 1.0")
    parser.add_option("-d","--dir",default="TESTDATA",help="The directory name (%default)")
    parser.add_option("-l","--dirperlevel",default="4",type="int",help="Number of directories created per level (%default)")
    parser.add_option("-f","--filesperdirectory",default="4",type="int",help="Number of files created per directory (%default)")
    parser.add_option("-t","--depth",default="3",type="int",help="Max depth (%default)")
    parser.add_option("--bdbh",default=os.getcwdu()+"/bdbh",help="path to the bdbh executable (%default)")
    parser.add_option("--random",default=os.getcwdu()+"/random_name",help="path to the random_name executable (%default)")
    parser.add_option("--testcase",default="1,1,-1",help="Your test case: 2,-1 means Adding 2 blocks and removing 1 block (%default)")
    (options, args) = parser.parse_args()

    testCase(options)

if __name__ == "__main__":
    main()

