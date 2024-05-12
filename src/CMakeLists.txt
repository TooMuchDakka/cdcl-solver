add_library(DimacsAbstractions INTERFACE)
target_include_directories(DimacsAbstractions
	INTERFACE
		${CMAKE_SOURCE_DIR}/include/dimacs/problemDefinition.hpp
)

add_library(DimacsParser "")
target_sources(DimacsParser
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/dimacsParser.cpp
		${CMAKE_SOURCE_DIR}/include/dimacs/localClauseLiteralRemover.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/localClauseLiteralRemover.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/dimacs/dimacsParser.hpp
)
target_include_directories(DimacsParser
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/dimacs
)
target_link_libraries(DimacsParser INTERFACE DimacsAbstractions)

add_library(BlockedClauseElimination "")
target_sources(BlockedClauseElimination
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/blockedClauseElimination.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/optimizations/blockedClauseElimination.hpp
)
target_include_directories(BlockedClauseElimination
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(BlockedClauseElimination PRIVATE DimacsAbstractions)

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
target_link_libraries(Solver PRIVATE DimacsParser BlockedClauseElimination)

add_executable(SolverExecutable main.cpp)
target_link_libraries(SolverExecutable PRIVATE Solver)