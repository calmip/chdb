cmake_minimum_required(VERSION 3.13)

project(chdb LANGUAGES CXX)

find_package(MPI REQUIRED COMPONENTS CXX)

if(NOT DEFINED CHDB_MPI)
   execute_process(COMMAND ./detect-mpi.sh OUTPUT_VARIABLE "CHDB_MPI" )
   Message(STATUS "MPI flavour detected = ${CHDB_MPI}")
else()
	Message(STATUS "MPI flavour forced = ${CHDB_MPI}")
endif()

add_executable(chdb.exe src/basicscheduler.cpp	src/chdb.cpp  src/directories.cpp src/parameters.cpp  src/scheduler.cpp  src/system.cpp  src/usingfs.cpp) 

target_link_libraries(chdb.exe PRIVATE MPI::MPI_CXX)


install(TARGETS chdb.exe DESTINATION bin)

if("${CHDB_MPI}" STREQUAL "OPENMPI")
	install(PROGRAMS  wrapper/openmpi/env-wrapper.sh DESTINATION bin)
	install(PROGRAMS  wrapper/openmpi/mod-wrapper.sh DESTINATION bin)
	install(PROGRAMS  wrapper/openmpi/slave-wrapper.sh DESTINATION bin)
	install(PROGRAMS  wrapper/openmpi/chdb DESTINATION bin)
elseif( "${CHDB_MPI}" STREQUAL "INTELMPI" )
        install(PROGRAMS  wrapper/intelmpi/env-wrapper.sh DESTINATION bin)
        install(PROGRAMS  wrapper/intelmpi/mod-wrapper.sh DESTINATION bin)
        install(PROGRAMS  wrapper/intelmpi/slave-wrapper.sh DESTINATION bin)
        install(PROGRAMS  wrapper/intelmpi/chdb DESTINATION bin)
else()
	Message(FATAL_ERROR "oooops, please call cmake with -DCHDB_MPI=INTELMPI or OPENMPI ")
endif()

add_custom_target(env.sh ALL sed "-e s#BINDIR#${CMAKE_INSTALL_PREFIX}/bin#" ${CMAKE_CURRENT_SOURCE_DIR}/env.sh.tmpl >env.sh COMMENT "Generating env.sh" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/env.sh.tmpl )


# TODO - To add tests
#      

