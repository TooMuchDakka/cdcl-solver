#ifndef BASE_BLOCKED_CLAUSE_ELIMINATION_TESTS_FIXTURE_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATION_TESTS_FIXTURE_HPP

#include <gtest/gtest.h>

class BaseBlockedClauseEliminationTestsFixture : public ::testing::Test
{
protected:
	void SetUp() override {
		GTEST_SKIP() << "Skipping all tests for this fixture";
	}
};

#endif