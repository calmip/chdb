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

#LDFLAGS = $(DEBUG) 
LDFLAGS = $(DEBUG) -static

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

bdbh.o: /usr/include/readline/readline.h /usr/include/stdio.h
bdbh.o: /usr/include/features.h /usr/include/stdc-predef.h
bdbh.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
bdbh.o: /usr/include/readline/rlstdc.h /usr/include/readline/rltypedefs.h
bdbh.o: /usr/include/readline/keymaps.h /usr/include/readline/chardefs.h
bdbh.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/xlocale.h
bdbh.o: /usr/include/string.h /usr/include/readline/tilde.h
bdbh.o: /usr/include/readline/history.h /usr/include/time.h
bdbh.o: /usr/include/stdlib.h /usr/include/alloca.h parameters.hpp
bdbh.o: constypes.hpp exception.hpp SimpleOpt.h command.hpp
bdbh.o: /usr/include/fcntl.h /usr/include/unistd.h /usr/include/getopt.h
bdbh.o: /usr/include/dirent.h /usr/include/errno.h /usr/include/db_cxx.h
bdbh.o: /usr/include/db.h /usr/include/inttypes.h /usr/include/stdint.h
bdbh.o: /usr/include/pthread.h /usr/include/sched.h create.hpp read.hpp
bdbh.o: write.hpp ls.hpp rm.hpp put.hpp chmod.hpp merge.hpp
parameters.o: /usr/include/unistd.h /usr/include/features.h
parameters.o: /usr/include/stdc-predef.h /usr/include/getopt.h
parameters.o: /usr/include/errno.h /usr/include/libgen.h
parameters.o: /usr/include/stdlib.h /usr/include/alloca.h command.hpp
parameters.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/xlocale.h
parameters.o: /usr/include/dirent.h constypes.hpp exception.hpp
parameters.o: /usr/include/string.h parameters.hpp SimpleOpt.h
parameters.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
parameters.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/libio.h
parameters.o: /usr/include/_G_config.h /usr/include/wchar.h
parameters.o: /usr/include/pthread.h /usr/include/endian.h
parameters.o: /usr/include/sched.h
create.o: /usr/include/time.h /usr/include/features.h
create.o: /usr/include/stdc-predef.h /usr/include/xlocale.h parameters.hpp
create.o: constypes.hpp exception.hpp /usr/include/string.h SimpleOpt.h
create.o: /usr/include/stdlib.h /usr/include/alloca.h command.hpp
create.o: /usr/include/fcntl.h /usr/include/unistd.h /usr/include/getopt.h
create.o: /usr/include/dirent.h /usr/include/errno.h /usr/include/db_cxx.h
create.o: /usr/include/db.h /usr/include/inttypes.h /usr/include/stdint.h
create.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
create.o: /usr/include/wchar.h /usr/include/pthread.h /usr/include/endian.h
create.o: /usr/include/sched.h create.hpp
command.o: /usr/include/stdlib.h /usr/include/features.h
command.o: /usr/include/stdc-predef.h /usr/include/alloca.h
command.o: /usr/include/signal.h /usr/include/time.h /usr/include/xlocale.h
command.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
command.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/libio.h
command.o: /usr/include/_G_config.h /usr/include/wchar.h
command.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/pthread.h
command.o: /usr/include/endian.h /usr/include/sched.h /usr/include/zlib.h
command.o: /usr/include/zconf.h /usr/include/limits.h parameters.hpp
command.o: constypes.hpp exception.hpp /usr/include/string.h SimpleOpt.h
command.o: command.hpp /usr/include/fcntl.h /usr/include/dirent.h
command.o: /usr/include/errno.h
read.o: /usr/include/time.h /usr/include/features.h
read.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
read.o: /usr/include/fcntl.h /usr/include/errno.h /usr/include/utime.h
read.o: parameters.hpp constypes.hpp exception.hpp /usr/include/string.h
read.o: SimpleOpt.h /usr/include/stdlib.h /usr/include/alloca.h command.hpp
read.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/dirent.h
read.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
read.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/libio.h
read.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/pthread.h
read.o: /usr/include/endian.h /usr/include/sched.h read.hpp
write.o: /usr/include/time.h /usr/include/features.h
write.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
write.o: /usr/include/fcntl.h /usr/include/dirent.h /usr/include/errno.h
write.o: /usr/include/utime.h parameters.hpp constypes.hpp exception.hpp
write.o: /usr/include/string.h SimpleOpt.h /usr/include/stdlib.h
write.o: /usr/include/alloca.h command.hpp /usr/include/unistd.h
write.o: /usr/include/getopt.h /usr/include/db_cxx.h /usr/include/db.h
write.o: /usr/include/inttypes.h /usr/include/stdint.h /usr/include/stdio.h
write.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
write.o: /usr/include/pthread.h /usr/include/endian.h /usr/include/sched.h
write.o: write.hpp
ls.o: /usr/include/time.h /usr/include/features.h /usr/include/stdc-predef.h
ls.o: /usr/include/xlocale.h /usr/include/pwd.h /usr/include/stdio.h
ls.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ls.o: /usr/include/grp.h /usr/include/fcntl.h /usr/include/errno.h
ls.o: /usr/include/utime.h parameters.hpp constypes.hpp exception.hpp
ls.o: /usr/include/string.h SimpleOpt.h /usr/include/stdlib.h
ls.o: /usr/include/alloca.h command.hpp /usr/include/unistd.h
ls.o: /usr/include/getopt.h /usr/include/dirent.h /usr/include/db_cxx.h
ls.o: /usr/include/db.h /usr/include/inttypes.h /usr/include/stdint.h
ls.o: /usr/include/pthread.h /usr/include/endian.h /usr/include/sched.h
ls.o: ls.hpp
rm.o: /usr/include/time.h /usr/include/features.h /usr/include/stdc-predef.h
rm.o: /usr/include/xlocale.h /usr/include/pwd.h /usr/include/stdio.h
rm.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
rm.o: /usr/include/grp.h /usr/include/fcntl.h /usr/include/errno.h
rm.o: /usr/include/utime.h parameters.hpp constypes.hpp exception.hpp
rm.o: /usr/include/string.h SimpleOpt.h /usr/include/stdlib.h
rm.o: /usr/include/alloca.h command.hpp /usr/include/unistd.h
rm.o: /usr/include/getopt.h /usr/include/dirent.h /usr/include/db_cxx.h
rm.o: /usr/include/db.h /usr/include/inttypes.h /usr/include/stdint.h
rm.o: /usr/include/pthread.h /usr/include/endian.h /usr/include/sched.h
rm.o: rm.hpp
put.o: /usr/include/time.h /usr/include/features.h /usr/include/stdc-predef.h
put.o: /usr/include/xlocale.h /usr/include/fcntl.h /usr/include/dirent.h
put.o: /usr/include/errno.h /usr/include/utime.h parameters.hpp constypes.hpp
put.o: exception.hpp /usr/include/string.h SimpleOpt.h /usr/include/stdlib.h
put.o: /usr/include/alloca.h command.hpp /usr/include/unistd.h
put.o: /usr/include/getopt.h /usr/include/db_cxx.h /usr/include/db.h
put.o: /usr/include/inttypes.h /usr/include/stdint.h /usr/include/stdio.h
put.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
put.o: /usr/include/pthread.h /usr/include/endian.h /usr/include/sched.h
put.o: put.hpp
chmod.o: /usr/include/time.h /usr/include/features.h
chmod.o: /usr/include/stdc-predef.h /usr/include/xlocale.h /usr/include/pwd.h
chmod.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
chmod.o: /usr/include/wchar.h /usr/include/grp.h /usr/include/fcntl.h
chmod.o: /usr/include/errno.h /usr/include/utime.h parameters.hpp
chmod.o: constypes.hpp exception.hpp /usr/include/string.h SimpleOpt.h
chmod.o: /usr/include/stdlib.h /usr/include/alloca.h command.hpp
chmod.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/dirent.h
chmod.o: /usr/include/db_cxx.h /usr/include/db.h /usr/include/inttypes.h
chmod.o: /usr/include/stdint.h /usr/include/pthread.h /usr/include/endian.h
chmod.o: /usr/include/sched.h chmod.hpp
merge.o: parameters.hpp constypes.hpp exception.hpp /usr/include/string.h
merge.o: /usr/include/features.h /usr/include/stdc-predef.h
merge.o: /usr/include/xlocale.h SimpleOpt.h /usr/include/stdlib.h
merge.o: /usr/include/alloca.h command.hpp /usr/include/fcntl.h
merge.o: /usr/include/time.h /usr/include/unistd.h /usr/include/getopt.h
merge.o: /usr/include/dirent.h /usr/include/errno.h /usr/include/db_cxx.h
merge.o: /usr/include/db.h /usr/include/inttypes.h /usr/include/stdint.h
merge.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
merge.o: /usr/include/wchar.h /usr/include/pthread.h /usr/include/endian.h
merge.o: /usr/include/sched.h merge.hpp
