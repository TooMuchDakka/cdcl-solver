# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (NOT TARGET gtest)
	add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest/ ${CMAKE_BINARY_DIR}/external/googletest EXCLUDE_FROM_ALL)
	set_target_properties(gtest gtest_main PROPERTIES FOLDER external)
endif()

add_executable(cdcl-tests "")
target_sources(cdcl-tests
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/dimacsParserTestFixture.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/dimacs/dimacsParserTests.cpp	

		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/avlIntervalTreeTestsFixture.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/baseBlockedClauseEliminationTestsFixture.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/opaqueAvlIntervalTree.hpp

		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/avlIntervalTreeTests.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/optimizations/blockedClauseEliminationTests.cpp
)
target_include_directories(cdcl-tests
	PRIVATE
		${CMAKE_SOURCE_DIR}/external/googletest/googletest/include
)
target_link_libraries(cdcl-tests DimacsAbstractions DimacsParser BlockedClauseElimination gtest gtest_main)
gtest_discover_tests(cdcl-tests)