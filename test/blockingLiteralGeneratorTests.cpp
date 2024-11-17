#include <gtest/gtest.h>

#include "dimacs/problemDefinition.hpp"
#include "gmock/gmock-matchers.h"
#include "optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp"

class BlockingLiteralGeneratorTests : public testing::Test {
public:
	static void assertGeneratedCandidateSequenceMatches(const std::initializer_list<long>& expectedCandidateSequence, blockedClauseElimination::BlockingLiteralGenerator& blockingLiteralGenerator)
	{
		std::optional<long> nextActualCandidate;
		for (const long expectedCandidate : expectedCandidateSequence)
		{
			ASSERT_NO_FATAL_FAILURE(nextActualCandidate = blockingLiteralGenerator.getNextCandiate());
			ASSERT_TRUE(nextActualCandidate.has_value());
			ASSERT_EQ(expectedCandidate, *nextActualCandidate);
		}
		ASSERT_NO_FATAL_FAILURE(nextActualCandidate = blockingLiteralGenerator.getNextCandiate());
		ASSERT_FALSE(nextActualCandidate.has_value());
	}

	static void initializeProblemDefinition(std::size_t numVariables, std::size_t numClauses, const std::initializer_list<std::initializer_list<long>>& clauseLiteralDefinitions, dimacs::ProblemDefinition::ptr& problemDefinition)
	{
		ASSERT_EQ(numClauses, clauseLiteralDefinitions.size());

		problemDefinition = std::make_shared<dimacs::ProblemDefinition>(numVariables, numClauses);
		std::size_t clauseIdx = 0;
		for (const std::initializer_list<long>& clauseLiterals : clauseLiteralDefinitions)
		{
			ASSERT_LT(0, clauseLiterals.size());
			ASSERT_TRUE(problemDefinition->addClause(clauseIdx++, dimacs::ProblemDefinition::Clause(clauseLiterals)));
		}
	}
};

TEST_F(BlockingLiteralGeneratorTests, SequentialCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 3;
	constexpr std::size_t numClausesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;

	ASSERT_NO_FATAL_FAILURE(initializeProblemDefinition(numVariablesInFormula, numClausesInFormula, {
		{1,2,3},
		{-3,-2,1},
		{1,-2,3}
	}, problemDefinition));

	auto blockingLiteralGenerator = blockedClauseElimination::BlockingLiteralGenerator::usingSequentialLiteralSelectionHeuristic();
	blockingLiteralGenerator->init({ -3,-2,1 }, problemDefinition->getLiteralOccurrenceLookup());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSequenceMatches({ -3,-2,1 }, *blockingLiteralGenerator));
}

TEST_F(BlockingLiteralGeneratorTests, MinimalClauseOverlapCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 5;
	constexpr std::size_t numClausesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;

	ASSERT_NO_FATAL_FAILURE(initializeProblemDefinition(numVariablesInFormula, numClausesInFormula, {
		{1,2,3,4,5},
		{-3,-2,1,-5},
		{1,-2,3,4}
		}, problemDefinition));

	auto blockingLiteralGenerator = blockedClauseElimination::BlockingLiteralGenerator::usingMinimumClauseOverlapForLiteralSelection();
	blockingLiteralGenerator->init({ -3,-2,1,-5 }, problemDefinition->getLiteralOccurrenceLookup());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSequenceMatches({ 1,-2,-5,-3 }, *blockingLiteralGenerator));
}

TEST_F(BlockingLiteralGeneratorTests, MaximumlClauseOverlapCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 5;
	constexpr std::size_t numClausesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;

	ASSERT_NO_FATAL_FAILURE(initializeProblemDefinition(numVariablesInFormula, numClausesInFormula, {
		{1,2,3,4,5},
		{-3,-2,1,-5},
		{1,-2,3,4}
		}, problemDefinition));

	auto blockingLiteralGenerator = blockedClauseElimination::BlockingLiteralGenerator::usingMaximumClauseOverlapForLiteralSelection();
	blockingLiteralGenerator->init({ -3,-2,1,-5 }, problemDefinition->getLiteralOccurrenceLookup());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSequenceMatches({ -3,-2,-5,1 }, *blockingLiteralGenerator));
}