cmake_minimum_required(VERSION 3.14)
project(cdcl-solver
	DESCRIPTION "A SAT solver utilizing the CDCL technique to prove SAT/UNSAT of a circuit in the DIMACS format"
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_CDCL_TESTS "Build the test suite" OFF)
option(BUILD_SET_BLOCKED_CLAUSE_BENCHMARK_EXECUTABLE "Build executable to perform benchmarking of set block clause elimination metric" OFF)
option(BUILD_SOLVER_EXECUTABLE "Build the cdcl solver executable" OFF)

include(cmake/ExternalDependencies.cmake)
add_subdirectory(src)

if(BUILD_CDCL_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test)
endif()