cmake_minimum_required(VERSION 3.6)
project(mpiTest C)

include_directories(SYSTEM ${MSMPI_INC})
find_package(MPI REQUIRED)

add_executable(mpiTest main.c)
