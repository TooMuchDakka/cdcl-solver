add_library(DimacsAbstractions "")
target_sources(DimacsAbstractions
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/literalOccurrenceLookup.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/problemDefinition.cpp
		${CMAKE_SOURCE_DIR}/include/optimizations/utils/binarySearchUtils.hpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/dimacs/literalInContainerIndexLookup.hpp
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

add_library(AvlIntervalTree "")
target_sources(AvlIntervalTree
	PRIVATE
		${CMAKE_SOURCE_DIR}/include/optimizations/utils/avlIntervalTreeNode.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/utils/avlIntervalTreeNode.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/utils/avlIntervalTree.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/optimizations/utils/avlIntervalTree.hpp
)
target_include_directories(AvlIntervalTree
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(AvlIntervalTree INTERFACE DimacsAbstractions)

add_library(SetBlockedClauseElimination "")
target_sources(SetBlockedClauseElimination
	PRIVATE
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.cpp
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.cpp
		
		${CMAKE_SOURCE_DIR}/include/optimizations/utils/clauseCandidateSelector.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/utils/clauseCandidateSelector.cpp
	PUBLIC
		${CMAKE_SOURCE_DIR}/include/optimizations/setBlockedClauseElimination/baseSetBlockedClauseEliminator.hpp
)
target_include_directories(SetBlockedClauseElimination
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(SetBlockedClauseElimination PRIVATE DimacsAbstractions AvlIntervalTree)

if(BUILD_SOLVER_EXECUTABLE)
	add_executable(SolverExecutable main.cpp)
	target_link_libraries(SolverExecutable PRIVATE DimacsParser SetBlockedClauseElimination)
endif()

if (BUILD_SET_BLOCKED_CLAUSE_BENCHMARK_EXECUTABLE)
	add_subdirectory(benchmark)
endif()