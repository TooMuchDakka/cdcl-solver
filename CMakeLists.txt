cmake_minimum_required(VERSION 3.9)
project(cdcl-solver
	DESCRIPTION "A SAT solver utilizing the CDCL technique to prove SAT/UNSAT of a circuit in the DIMACS format"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)