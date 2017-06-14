#
# Makefile
#

#
# The flags for using the gcc compiler (with flags for either
# debugging and error checking the program or optimizing the code).
# Do NOT specify any optimisation level (some strange things happen and this is probably useless)
#

##INCLUDES=-I ~/include
CXXFLAGS=-DHAVE_CXX_STDHEADERS $(DEBUG) $(INCLUDES) -Wall -pedantic

# POUR CAT UTILISER LES VARIABLES COMMENTEES

LDFLAGS = $(DEBUG) 
#LDFLAGS = $(DEBUG) -static

# The exec

EXECFILE= bdbh 

#
# The source files, object files, libraries and executable name.
#

SRCFILES = bdbh.cpp parameters.cpp create.cpp command.cpp read.cpp write.cpp ls.cpp rm.cpp put.cpp chmod.cpp merge.cpp

OBJFILES = parameters.o create.o command.o read.o write.o ls.o rm.o put.o chmod.o merge.o

LIBFILE  = libbdbh.a

# Sur les debian recentes il faut mettre -ltinfo et pas -lcurses !
#LIBS= -ldb_cxx -lpthread -ldb -lz -lreadline -lcurses
###LIB_DIRECTORY=-L ~/lib
LIBS=$(LIB_DIRECTORY) -ldb_cxx -lpthread -ldb -lreadline -ltinfo -lz

#
# The implicit rules
#

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) -c $<
#
# The make rule for the executable
#

all : bdbh

libbdbh.a : $(OBJFILES)
	ar ru $@ $^

bdbh : $(LIBFILE) bdbh.o
	$(CXX) $(CXXFLAGS) -o bdbh bdbh.o -L. -lbdbh $(LDFLAGS) $(LIBS)

random_name: random_name.o
	$(CXX) $(LDFLAGS) -o random_name -lrt $< 


#
# The tests
#
test: random_name bdbh
	./test.sh


#
# The dependencies for each of the *.o files.
#
depend: 
	makedepend -- $(CFLAGS) -- $(SRCFILES)

#
# Other Standard make rules
#

clean:
	rm -f *.o *.a *.so *~ $(EXECFILE)

distclean: clean
	rm -f $(EXECFILE)

remove: distclean

objfiles: $(OBJFILES)

# DO NOT DELETE
