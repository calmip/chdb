#
# Makefile for chdb
#

DEBUG=-g
CXX=mpicxx

#
# The flags for using the gcc compiler (with flags for either
# debugging and error checking the program or optimizing the code).
#

CXXFLAGS=$(DEBUG) -Wall -pedantic

# POUR CAT UTILISER LES VARIABLES COMMENTEES

LDFLAGS=$(DEBUG)

# The exec

EXECFILE=chdb 

#
# The source files, object files, libraries and executable name.
#

SRCFILES=usingfs.cpp directories.cpp parameters.cpp chdb.cpp scheduler.cpp basicscheduler.cpp

OBJFILES=usingfs.o parameters.o chdb.o directories.o scheduler.o basicscheduler.o

LIBS=

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
# The tests
#
test: random_name bdbh
	./test.sh


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

usingfs.o: /usr/include/sys/types.h /usr/include/features.h
usingfs.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
usingfs.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
usingfs.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
usingfs.o: /usr/include/time.h /usr/include/endian.h
usingfs.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
usingfs.o: /usr/include/sys/select.h /usr/include/bits/select.h
usingfs.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
usingfs.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
usingfs.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
usingfs.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
usingfs.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
usingfs.o: /usr/include/getopt.h usingfs.hpp constypes.hpp directories.hpp
usingfs.o: parameters.hpp SimpleOpt.h /usr/include/stdlib.h
usingfs.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
usingfs.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
usingfs.o: /usr/include/libgen.h /usr/include/dirent.h
usingfs.o: /usr/include/bits/dirent.h /usr/include/bits/posix1_lim.h
usingfs.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
directories.o: /usr/include/sys/types.h /usr/include/features.h
directories.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
directories.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
directories.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
directories.o: /usr/include/time.h /usr/include/endian.h
directories.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
directories.o: /usr/include/sys/select.h /usr/include/bits/select.h
directories.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
directories.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
directories.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
directories.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
directories.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
directories.o: /usr/include/getopt.h /usr/include/libgen.h usingfs.hpp
directories.o: constypes.hpp directories.hpp parameters.hpp SimpleOpt.h
directories.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
directories.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
directories.o: /usr/include/string.h /usr/include/xlocale.h
directories.o: /usr/include/dirent.h /usr/include/bits/dirent.h
directories.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
directories.o: /usr/include/linux/limits.h
parameters.o: /usr/include/sys/types.h /usr/include/features.h
parameters.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
parameters.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
parameters.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
parameters.o: /usr/include/time.h /usr/include/endian.h
parameters.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
parameters.o: /usr/include/sys/select.h /usr/include/bits/select.h
parameters.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
parameters.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
parameters.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
parameters.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
parameters.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
parameters.o: /usr/include/getopt.h parameters.hpp SimpleOpt.h
parameters.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
parameters.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
parameters.o: /usr/include/string.h /usr/include/xlocale.h constypes.hpp
chdb.o: parameters.hpp SimpleOpt.h /usr/include/stdlib.h
chdb.o: /usr/include/features.h /usr/include/sys/cdefs.h
chdb.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
chdb.o: /usr/include/gnu/stubs-64.h /usr/include/bits/waitflags.h
chdb.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
chdb.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
chdb.o: /usr/include/sys/types.h /usr/include/bits/types.h
chdb.o: /usr/include/bits/typesizes.h /usr/include/time.h
chdb.o: /usr/include/sys/select.h /usr/include/bits/select.h
chdb.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
chdb.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
chdb.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
chdb.o: constypes.hpp usingfs.hpp directories.hpp basicscheduler.hpp
chdb.o: scheduler.hpp
scheduler.o: /usr/include/sys/types.h /usr/include/features.h
scheduler.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
scheduler.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
scheduler.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
scheduler.o: /usr/include/time.h /usr/include/endian.h
scheduler.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
scheduler.o: /usr/include/sys/select.h /usr/include/bits/select.h
scheduler.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
scheduler.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
scheduler.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
scheduler.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
scheduler.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
scheduler.o: /usr/include/getopt.h scheduler.hpp constypes.hpp parameters.hpp
scheduler.o: SimpleOpt.h /usr/include/stdlib.h /usr/include/bits/waitflags.h
scheduler.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
scheduler.o: /usr/include/string.h /usr/include/xlocale.h directories.hpp
scheduler.o: /usr/include/dirent.h /usr/include/bits/dirent.h
scheduler.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
scheduler.o: /usr/include/linux/limits.h
basicscheduler.o: /usr/include/assert.h /usr/include/features.h
basicscheduler.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
basicscheduler.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
basicscheduler.o: /usr/include/sys/types.h /usr/include/bits/types.h
basicscheduler.o: /usr/include/bits/typesizes.h /usr/include/time.h
basicscheduler.o: /usr/include/endian.h /usr/include/bits/endian.h
basicscheduler.o: /usr/include/bits/byteswap.h /usr/include/sys/select.h
basicscheduler.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
basicscheduler.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
basicscheduler.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/stat.h
basicscheduler.o: /usr/include/bits/stat.h /usr/include/unistd.h
basicscheduler.o: /usr/include/bits/posix_opt.h
basicscheduler.o: /usr/include/bits/environments.h
basicscheduler.o: /usr/include/bits/confname.h /usr/include/getopt.h
basicscheduler.o: basicscheduler.hpp constypes.hpp scheduler.hpp
basicscheduler.o: parameters.hpp SimpleOpt.h /usr/include/stdlib.h
basicscheduler.o: /usr/include/bits/waitflags.h
basicscheduler.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
basicscheduler.o: /usr/include/string.h /usr/include/xlocale.h
basicscheduler.o: directories.hpp /usr/include/dirent.h
basicscheduler.o: /usr/include/bits/dirent.h /usr/include/bits/posix1_lim.h
basicscheduler.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
