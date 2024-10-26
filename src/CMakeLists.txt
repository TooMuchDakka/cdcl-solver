add_library(DimacsAbstractions "")
target_sources(DimacsAbstractions
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/literalOccurrenceLookup.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/dimacs/problemDefinition.hpp	
		${CMAKE_SOURCE_DIR}/include/dimacs/literalOccurrenceLookup.hpp
)
target_include_directories(DimacsAbstractions
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)

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
target_link_libraries(DimacsParser INTERFACE DimacsAbstractions)

add_library(SetBlockedClauseElimination "")
target_sources(SetBlockedClauseElimination
	PRIVATE
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.cpp
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/baseSetBlockedClauseEliminator.hpp
)
target_include_directories(SetBlockedClauseElimination
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(SetBlockedClauseElimination PRIVATE DimacsAbstractions)

add_executable(SolverExecutable main.cpp)
target_link_libraries(SolverExecutable PRIVATE DimacsParser SetBlockedClauseElimination)