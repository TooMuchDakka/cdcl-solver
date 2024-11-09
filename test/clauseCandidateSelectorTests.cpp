#include <gtest/gtest.h>

#include "optimizations/utils/clauseCandidateSelector.hpp"

using namespace clauseCandidateSelection;

class ClauseCandidateSelectorTests : public testing::Test {
public:
	static void addClausesToFormula(dimacs::ProblemDefinition& cnfFormulaContainer, const std::initializer_list<std::initializer_list<long>>& formulaClauses)
	{
		std::size_t clauseIdx = 0;
		for (const auto& clauseLiterals : formulaClauses)
			ASSERT_TRUE(cnfFormulaContainer.addClause(clauseIdx++, dimacs::ProblemDefinition::Clause(clauseLiterals)));
	}

	static void assertGeneratedCandidateSeqeuenceMatches(ClauseCandidateSelector& clauseCandidateSelector, const std::initializer_list<std::size_t>& expectedCandidateClauseIndices)
	{
		std::optional<std::size_t> generatedCandidateIndex;
		for (const std::size_t expectedCandidateClauseIndx : expectedCandidateClauseIndices)
		{
			ASSERT_NO_FATAL_FAILURE(generatedCandidateIndex = clauseCandidateSelector.selectNextCandidate());
			ASSERT_TRUE(generatedCandidateIndex.has_value());
			ASSERT_EQ(expectedCandidateClauseIndx, generatedCandidateIndex);
		}
		ASSERT_FALSE(clauseCandidateSelector.selectNextCandidate());
	}
};

TEST_F(ClauseCandidateSelectorTests, SequentialClauseSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 3;
	constexpr std::size_t numClausesInFormula = 4;
	dimacs::ProblemDefinition problemDefinition(numVariablesInFormula, numClausesInFormula);
	ASSERT_NO_FATAL_FAILURE(addClausesToFormula(problemDefinition, { {1,2,3}, {1,2}, {-2, -3}, {-1,2} }));

	auto clauseCandidateSelector = ClauseCandidateSelector::initUsingSequentialCandidateSelection(problemDefinition.getNumClausesAfterOptimizations());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSeqeuenceMatches(*clauseCandidateSelector, { 0,1,2,3 }));
}

TEST_F(ClauseCandidateSelectorTests, MinimumClauseOverlapCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 3;
	constexpr std::size_t numClausesInFormula = 4;
	dimacs::ProblemDefinition problemDefinition(numVariablesInFormula, numClausesInFormula);
	ASSERT_NO_FATAL_FAILURE(addClausesToFormula(problemDefinition, { {1,2,3}, {1,2}, {-2, -3}, {1,2,-3} }));
	// 0: 2 | 1: 1 | 2: 3 | 3: 2

	auto clauseCandidateSelector = ClauseCandidateSelector::initUsingMinimalClauseOverlapForCandidateSelection(problemDefinition);
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSeqeuenceMatches(*clauseCandidateSelector, { 1,0,3,2 }));
}

TEST_F(ClauseCandidateSelectorTests, MaximumClauseOverlapCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 3;
	constexpr std::size_t numClausesInFormula = 5;
	dimacs::ProblemDefinition problemDefinition(numVariablesInFormula, numClausesInFormula);
	ASSERT_NO_FATAL_FAILURE(addClausesToFormula(problemDefinition, { {1,2}, {-1, -2, -3}, {-1,-2,3}, {1,2,-3}, {2, 3} }));
	// 0: 2 | 1: 4 | 2: 4 | 3: 4 | 4: 3

	auto clauseCandidateSelector = ClauseCandidateSelector::initUsingMaximalClauseOverlapForCandidateSelection(problemDefinition);
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSeqeuenceMatches(*clauseCandidateSelector, { 1, 2, 3, 4, 0 }));
}

TEST_F(ClauseCandidateSelectorTests, MinimumClauseLengthCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 4;
	constexpr std::size_t numClausesInFormula = 5;
	dimacs::ProblemDefinition problemDefinition(numVariablesInFormula, numClausesInFormula);
	ASSERT_NO_FATAL_FAILURE(addClausesToFormula(problemDefinition, { {1,2,3}, {1,2}, {1,2,3,4}, { -2, -3 }, {1,2,-3} }));

	auto clauseCandidateSelector = ClauseCandidateSelector::initUsingMinimumClauseLenghtForCandidateSelection(problemDefinition);
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSeqeuenceMatches(*clauseCandidateSelector, { 1,3,0,4,2 }));
}

TEST_F(ClauseCandidateSelectorTests, MaximumClauseLengthCandidateSelectionHeuristic)
{
	constexpr std::size_t numVariablesInFormula = 4;
	constexpr std::size_t numClausesInFormula = 5;
	dimacs::ProblemDefinition problemDefinition(numVariablesInFormula, numClausesInFormula);
	ASSERT_NO_FATAL_FAILURE(addClausesToFormula(problemDefinition, { {1,2,3}, {1,2}, {1,2,3,4}, { -2, -3 }, {1,2,-3} }));

	auto clauseCandidateSelector = ClauseCandidateSelector::initUsingMaximumClauseLengthForCandidateSelection(problemDefinition);
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidateSeqeuenceMatches(*clauseCandidateSelector, { 2,0,4,1,3 }));
}