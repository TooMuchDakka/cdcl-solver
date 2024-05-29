#ifndef BASE_BLOCKED_CLAUSE_ELIMINATION_TESTS_FIXTURE_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATION_TESTS_FIXTURE_HPP

#include <gtest/gtest.h>
#include "optimizations/baseBlockedClauseEliminator.hpp"

class BaseBlockedClauseEliminationTestsFixture : public ::testing::Test
{
protected:
	const blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult NO_BLOCKED_CLAUSE_FOUND_SEARCH_RESULT = { false, 0 };

	void SetUp() override {
		//GTEST_SKIP() << "Skipping all tests for this fixture";
	}
};

#endif