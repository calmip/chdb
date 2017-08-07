#
# Makefile for chdb
#

DEBUG=-g
CXX=mpicxx

#
# The flags for using the gcc compiler (with flags for either
# debugging and error checking the program or optimizing the code).
#

CXXFLAGS=$(DEBUG) -Wall -pedantic -std=c++11 -Wno-deprecated-declarations
LDFLAGS=$(DEBUG)

# The exec

EXECFILE=chdb.exe

#
# The source files, object files, libraries and executable name.
#

SRCFILES=usingbdbh.cpp usingfs.cpp directories.cpp parameters.cpp chdb.cpp scheduler.cpp basicscheduler.cpp system.cpp

OBJFILES=usingbdbh.o usingfs.o parameters.o chdb.o directories.o scheduler.o basicscheduler.o system.o

LIBS=-L bdbh -lbdbh -ldb_cxx -ldb -lz

#
# The implicit rules
#

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) -c $<
#
# The make rule for the executable
#

all : chdb

chdb : $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $(EXECFILE) $(OBJFILES) $(LIBS) $(LDFLAGS) 


#
# Other Standard make rules
#

clean:
	rm -f *.o *~ $(EXECFILE)

distclean: clean
	rm -f $(EXECFILE)

remove: distclean

depend: 
	makedepend -- $(CFLAGS) -- $(SRCFILES)

objfiles: $(OBJFILES)
# DO NOT DELETE
