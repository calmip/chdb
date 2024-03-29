cmake_minimum_required(VERSION 3.13)

project(chdb LANGUAGES CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

Message (STATUS "CMAKE_BUILD_TYPE     = " ${CMAKE_BUILD_TYPE})
Message (STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX})
Message (STATUS "CMAKE_CXX_STANDARD   = " ${CMAKE_CXX_STANDARD})

add_compile_options(
#       -Wall -Werror
       -Wall
       "$<$<CONFIG:RELEASE>:-O3>"
       "$<$<CONFIG:DEBUG>:-O0>"
       "$<$<CONFIG:DEBUG>:-g>"
)

#add_compile_options(-Wall -Werror -O3)

find_package(MPI REQUIRED COMPONENTS CXX)

if(NOT DEFINED CHDB_MPI)
   execute_process(COMMAND ./detect-mpi.sh OUTPUT_VARIABLE "CHDB_MPI" )
   Message(STATUS "MPI flavour detected = ${CHDB_MPI}")
else()
    Message(STATUS "MPI flavour forced = ${CHDB_MPI}")
endif()

add_executable(chdb.exe src/chdb.cpp src/basicscheduler.cpp src/directories.cpp src/parameters.cpp src/scheduler.cpp src/system.cpp src/usingfs.cpp)
#target_link_libraries(chdb.exe PRIVATE chdb mpi)
target_link_libraries(chdb.exe PRIVATE MPI::MPI_CXX)
install(TARGETS chdb.exe DESTINATION bin)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    if(GOOGLETEST_ROOT)
        Message (STATUS "GOOGLETEST_ROOT = " ${GOOGLETEST_ROOT})
        include_directories(${GOOGLETEST_ROOT}/include)
        link_directories(${GOOGLETEST_ROOT}/lib64)
    endif()
    add_executable(parameters_unittest ${GOOGLETEST_ROOT}/googletest/src/gtest_main.cc gtests/parameters_unittest.cpp gtests/constypes_unittest.cpp src/system.cpp src/parameters.cpp)
    add_executable(system_unittest ${GOOGLETEST_ROOT}/googletest/src/gtest_main.cc gtests/system_unittest.cpp gtests/constypes_unittest.cpp src/system.cpp)
    add_executable(directories_unittest ${GOOGLETEST_ROOT}/googletest/src/gtest_main.cc gtests/directories_unittest.cpp gtests/constypes_unittest.cpp src/system.cpp src/usingfs.cpp src/directories.cpp src/parameters.cpp)
    add_executable(scheduler_unittest ${GOOGLETEST_ROOT}/googletest/src/gtest_main.cc gtests/scheduler_unittest.cpp gtests/constypes_unittest.cpp src/basicscheduler.cpp src/scheduler.cpp src/system.cpp src/usingfs.cpp src/directories.cpp src/parameters.cpp)
    target_link_libraries(parameters_unittest PRIVATE MPI::MPI_CXX gtest)
    target_link_libraries(system_unittest PRIVATE MPI::MPI_CXX gtest)
    target_link_libraries(directories_unittest PRIVATE MPI::MPI_CXX gtest)
    target_link_libraries(scheduler_unittest PRIVATE MPI::MPI_CXX gtest)
    add_custom_target(ext_cmd ALL cp ${CMAKE_CURRENT_SOURCE_DIR}/gtests/ext_cmd.sh . COMMENT "Generating ext_cmd.sh"  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gtests/ext_cmd.sh )
endif()

install(PROGRAMS  wrapper/slave-openmpi-wrapper.sh DESTINATION bin)
install(PROGRAMS  wrapper/slave-intelmpi-wrapper.sh DESTINATION bin)
install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/slave--wrapper.sh DESTINATION bin)
install(PROGRAMS  wrapper/env-wrapper.sh DESTINATION bin)
install(PROGRAMS  wrapper/snippet-wrapper.sh DESTINATION bin)
install(PROGRAMS  wrapper/mod-wrapper.sh DESTINATION bin)
install(PROGRAMS  wrapper/chdb DESTINATION bin)

# NOTE - The touch is a dirty trick, if you don't do that slave--wrapper.sh cannot be installed 
if("${CHDB_MPI}" STREQUAL "OPENMPI")
    add_custom_target( slave--wrapper.sh ALL COMMAND touch slave-openmpi-wrapper.sh && ln -sf slave-openmpi-wrapper.sh slave--wrapper.sh COMMENT "Creating symbolic link slave--wrapper.sh" )
elseif( "${CHDB_MPI}" STREQUAL "INTELMPI" )
    add_custom_target( slave--wrapper.sh ALL COMMAND touch slave-intelmpi-wrapper.sh && ln -sf slave-intelmpi-wrapper.sh slave--wrapper.sh COMMENT "Creating symbolic link slave--wrapper.sh")
else()
    Message(FATAL_ERROR "oooops, please call cmake with -DCHDB_MPI=INTELMPI or OPENMPI ")
endif()

add_custom_target(env.sh ALL sed "-e s#BINDIR#${CMAKE_INSTALL_PREFIX}/bin#g" ${CMAKE_CURRENT_SOURCE_DIR}/env.sh.tmpl >env.sh COMMENT "Generating env.sh" DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/env.sh.tmpl )
Message (STATUS "env.sh will be generated ! Please have a look to this file")

# TODO - To add tests
#      

