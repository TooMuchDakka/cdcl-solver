add_library(DimacsParser "")
target_sources(DimacsParser
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/dimacsParser.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/dimacs/dimacsParser.hpp
)
target_include_directories(DimacsParser
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)


add_library(Solver "")
target_sources(Solver
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/solver/solver.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/solver/solver.hpp
)
target_include_directories(Solver
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(Solver PRIVATE DimacsParser)

add_executable(SolverExecutable main.cpp)
target_link_libraries(SolverExecutable PRIVATE Solver)