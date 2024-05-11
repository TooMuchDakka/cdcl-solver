#ifndef DIMACS_PARSER_TEST_FIXTURE_HPP
#define DIMACS_PARSER_TEST_FIXTURE_HPP

#include <gtest/gtest.h>

class DimacsParserTestFixture : public ::testing::Test
{
protected:
	void SetUp() override {
		GTEST_SKIP() << "Skipping all tests for this fixture";
	}
};

#endif