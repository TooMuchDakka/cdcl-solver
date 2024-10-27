file(GLOB_RECURSE CDCL_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# create an executable in which the tests will be stored
add_executable(
  cdclTests
  ${CDCL_TEST_SOURCES}
)
# link the Google test infrastructure and a default main function to the test executable.
target_link_libraries(cdclTests PRIVATE gmock gtest_main SetBlockedClauseElimination DimacsAbstractions)

gtest_discover_tests(cdclTests)