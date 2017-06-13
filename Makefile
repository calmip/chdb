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

usingbdbh.o: /usr/include/sys/types.h /usr/include/features.h
usingbdbh.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
usingbdbh.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
usingbdbh.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
usingbdbh.o: /usr/include/time.h /usr/include/endian.h
usingbdbh.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
usingbdbh.o: /usr/include/sys/select.h /usr/include/bits/select.h
usingbdbh.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
usingbdbh.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
usingbdbh.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
usingbdbh.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
usingbdbh.o: /usr/include/bits/environments.h /usr/include/bits/confname.h
usingbdbh.o: /usr/include/getopt.h system.hpp constypes.hpp usingbdbh.hpp
usingbdbh.o: directories.hpp parameters.hpp SimpleOpt.h /usr/include/stdlib.h
usingbdbh.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
usingbdbh.o: /usr/include/alloca.h /usr/include/string.h
usingbdbh.o: /usr/include/xlocale.h bdbh/ls.hpp bdbh/command.hpp
usingbdbh.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
usingbdbh.o: /usr/include/dirent.h /usr/include/bits/dirent.h
usingbdbh.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
usingbdbh.o: /usr/include/linux/limits.h /usr/include/errno.h
usingbdbh.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
usingbdbh.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
usingbdbh.o: /usr/include/asm-generic/errno-base.h bdbh/exception.hpp
usingbdbh.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
usingbdbh.o: /usr/include/stdint.h /usr/include/bits/wchar.h
usingbdbh.o: /usr/include/stdio.h /usr/include/libio.h
usingbdbh.o: /usr/include/_G_config.h /usr/include/wchar.h
usingbdbh.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
usingbdbh.o: /usr/include/pthread.h /usr/include/sched.h
usingbdbh.o: /usr/include/bits/sched.h /usr/include/bits/setjmp.h
usingbdbh.o: /usr/include/libgen.h
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
usingfs.o: /usr/include/getopt.h system.hpp constypes.hpp usingfs.hpp
usingfs.o: directories.hpp parameters.hpp SimpleOpt.h /usr/include/stdlib.h
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
directories.o: /usr/include/getopt.h usingfs.hpp constypes.hpp
directories.o: directories.hpp parameters.hpp SimpleOpt.h
directories.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
directories.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
directories.o: /usr/include/string.h /usr/include/xlocale.h
directories.o: /usr/include/dirent.h /usr/include/bits/dirent.h
directories.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
directories.o: /usr/include/linux/limits.h system.hpp
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
parameters.o: system.hpp
chdb.o: system.hpp constypes.hpp parameters.hpp SimpleOpt.h
chdb.o: /usr/include/stdlib.h /usr/include/features.h
chdb.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
chdb.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
chdb.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
chdb.o: /usr/include/endian.h /usr/include/bits/endian.h
chdb.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
chdb.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
chdb.o: /usr/include/time.h /usr/include/sys/select.h
chdb.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
chdb.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
chdb.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
chdb.o: /usr/include/string.h /usr/include/xlocale.h usingfs.hpp
chdb.o: directories.hpp usingbdbh.hpp bdbh/ls.hpp bdbh/command.hpp
chdb.o: /usr/include/sys/stat.h /usr/include/bits/stat.h /usr/include/fcntl.h
chdb.o: /usr/include/bits/fcntl.h /usr/include/unistd.h
chdb.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
chdb.o: /usr/include/bits/confname.h /usr/include/getopt.h
chdb.o: /usr/include/dirent.h /usr/include/bits/dirent.h
chdb.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
chdb.o: /usr/include/linux/limits.h /usr/include/errno.h
chdb.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
chdb.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
chdb.o: /usr/include/asm-generic/errno-base.h bdbh/exception.hpp
chdb.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
chdb.o: /usr/include/stdint.h /usr/include/bits/wchar.h /usr/include/stdio.h
chdb.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
chdb.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
chdb.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
chdb.o: /usr/include/bits/setjmp.h basicscheduler.hpp scheduler.hpp
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
basicscheduler.o: system.hpp constypes.hpp basicscheduler.hpp scheduler.hpp
basicscheduler.o: parameters.hpp SimpleOpt.h /usr/include/stdlib.h
basicscheduler.o: /usr/include/bits/waitflags.h
basicscheduler.o: /usr/include/bits/waitstatus.h /usr/include/alloca.h
basicscheduler.o: /usr/include/string.h /usr/include/xlocale.h
basicscheduler.o: directories.hpp /usr/include/dirent.h
basicscheduler.o: /usr/include/bits/dirent.h /usr/include/bits/posix1_lim.h
basicscheduler.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
system.o: /usr/include/sys/types.h /usr/include/features.h
system.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
system.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
system.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
system.o: /usr/include/time.h /usr/include/endian.h
system.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
system.o: /usr/include/sys/select.h /usr/include/bits/select.h
system.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
system.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
system.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
system.o: /usr/include/libgen.h system.hpp constypes.hpp
