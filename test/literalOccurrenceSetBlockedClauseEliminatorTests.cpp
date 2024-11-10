#include <gtest/gtest.h>
#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>

using namespace setBlockedClauseElimination;

class LiteralOccurrenceSetBlockedClauseEliminatorTests : public testing::Test {
public:
	static void generateProblemDefinition(const std::size_t numVariablesInFormula, const std::initializer_list<std::initializer_list<long>>& clausesOfFormula, dimacs::ProblemDefinition::ptr& problemDefinition)
	{
		ASSERT_GT(numVariablesInFormula, 0);
		ASSERT_GT(clausesOfFormula.size(), 0);

		problemDefinition = std::make_shared<dimacs::ProblemDefinition>(numVariablesInFormula, clausesOfFormula.size());
		ASSERT_TRUE(problemDefinition);

		std::size_t clauseIdx = 0;
		for (const auto literalsOfClause : clausesOfFormula)
			problemDefinition->addClause(clauseIdx++, dimacs::ProblemDefinition::Clause(std::vector(literalsOfClause.begin(), literalsOfClause.end())));
	}

	static void assertThatBlockingSetsMatchOrThrow(const BaseSetBlockedClauseEliminator::FoundBlockingSet& expected, const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet>& actual)
	{
		ASSERT_TRUE(actual.has_value());
		ASSERT_EQ(expected.size(), actual->size());

		if (expected.empty())
			return;

		const std::unordered_set<long> expectedLiteralsOfBlockingSet(expected.cbegin(), expected.cend());
		const BaseSetBlockedClauseEliminator::FoundBlockingSet& actualLiteralsOfBlockingSet = *actual;
		for (const long literal : actualLiteralsOfBlockingSet)
			ASSERT_TRUE(expectedLiteralsOfBlockingSet.count(literal));
	}

	static void generateBlockingSetCandidateGeneratorOrThrow(LiteralOccurrenceBlockingSetCandidateGenerator::ptr& blockingSetCandidateGenerator)
	{
		blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
		ASSERT_TRUE(blockingSetCandidateGenerator);
	}
};

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, EmptyResolutionEnvironmentDoesNotCauseClauseToBeSetBlocked)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(3, { {1,2}, {1,2,3}, {1,2} }, problemDefinition));

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 0;
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);
	ASSERT_FALSE(setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator));
}

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, NotAllClausesInResolutionEnvironmentMatchingCausesClauseToBeNotSetBlocked)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(2, { {1,-2}, {1,2}, {-1,-2} }, problemDefinition));

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 0;
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);
	ASSERT_FALSE(setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 1,1 })));
}

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, BlockingSetOfSizeOneCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(2, { {1, -2}, {-1, 2}, {-1, -2} }, problemDefinition));
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 0;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -2 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator);
	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
}

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, BlockingSetOfSizeLargerThanOneCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(5, { {1, 2, 3, 4, 5}, {1, -2, 3}, {-1, 2, 3}, {-1, -2, -3, 4, -5 }}, problemDefinition));
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 3;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -1, -2, -5 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 }));
	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
}

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, TautologyBetweenDifferenceSetOfProcessedClauseAndClauseOfResolutionEnvironmentCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(3, { {-3, -2, 1}, {-1, 2, -3} }, problemDefinition));
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 0;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -2 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator);
	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
}

// TODO: Whether we can assume that local variables, variables only used in one polarity in the cnf, where propagated in the cnf during processing effects the detection of blocking sets
//TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, TautologyBetweenClauseOfResolutionEnvironmentAndNotOverlappingPartsOfBlockingSetCorrectlyDetected)
//{
//	dimacs::ProblemDefinition::ptr problemDefinition;
//	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(4, { {-1, 2, 3, 4}, {1, -2, -3}, {1, -2, 3, -4} }, problemDefinition));
//	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);
//
//	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
//	ASSERT_TRUE(blockingSetCandidateGenerator);
//
//	constexpr std::size_t clauseToBeChecked = 2;
//	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { 1, -2, 3 };
//	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 }));
//	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
//}

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, EmptyUnionOfClauseLiteralsAndBlockingSetNotDetectedAsTautology)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(3, { {1, -2, 3}, {-1, 2, -3} }, problemDefinition));
	auto setBlockedClauseEliminatorInstance = LiteralOccurrenceSetBlockedClauseEliminator(problemDefinition);

	const auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_TRUE(blockingSetCandidateGenerator);

	constexpr std::size_t clauseToBeChecked = 0;
	ASSERT_FALSE(setBlockedClauseEliminatorInstance.determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 })));
}