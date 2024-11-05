#include <gtest/gtest.h>
#include <optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp>

using namespace setBlockedClauseElimination;

class LiteralOccurrenceBlockingSetCandidateGeneratorTests : public testing::Test {
public:
	using BlockingSetCandidateCollection = std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate>;
	
	[[nodiscard]] static BlockingSetCandidateCollection buildExpectedCandidateLookup(const std::initializer_list<std::initializer_list<long>>& expectedCandidates)
	{
		BlockingSetCandidateCollection candidateContainer;
		candidateContainer.reserve(expectedCandidates.size());

		for (const std::initializer_list<long>& candidateData : expectedCandidates)
		{
			BaseBlockingSetCandidateGenerator::BlockingSetCandidate candidate;
			for (const long literal : candidateData)
				candidate.emplace(literal);
			candidateContainer.emplace_back(candidate);
		}
		return candidateContainer;
	}

	[[nodiscard]] static std::string stringifyBlockingSet(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& blockingSet)
	{
		EXPECT_GT(blockingSet.size(), 2);

		std::vector<long> blockingSetLiterals;
		blockingSetLiterals.insert(blockingSetLiterals.end(), blockingSet.cbegin(), blockingSet.cend());

		std::stringstream stringifiedContentContainer;
		for (std::size_t i = 0; i < blockingSetLiterals.size() - 1; ++i)
			stringifiedContentContainer << std::to_string(blockingSetLiterals.at(i)) << ",";
		stringifiedContentContainer << blockingSetLiterals.back();

		return stringifiedContentContainer.str();
	}

	[[nodiscard]] static bool areBlockingSetsEqual(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& expected, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& actual)
	{
		return expected.size() == actual.size()
			&& std::all_of(
			expected.cbegin(),
			expected.cend(),
			[&actual](const long literal)
			{
				return actual.count(literal);
			});
	}

	static void assertGeneratedCandidatesAreEqual(const BlockingSetCandidateCollection& expectedCandidates, const BlockingSetCandidateCollection& actualCandidates)
	{
		ASSERT_EQ(expectedCandidates.size(), actualCandidates.size());
		for (std::size_t i = 0; i < expectedCandidates.size(); ++i)
			ASSERT_TRUE(areBlockingSetsEqual(expectedCandidates.at(i), actualCandidates.at(i))) << "Expected candidate: " << stringifyBlockingSet(expectedCandidates.at(i)) << " but was actually: " << stringifyBlockingSet(actualCandidates.at(i));
	}

	static void determineActualCandidatesOrThrow(BaseBlockingSetCandidateGenerator& candidateGenerator, std::size_t numExpectedCandidates, BlockingSetCandidateCollection& actualCandidates)
	{
		actualCandidates.reserve(numExpectedCandidates);
		for (std::size_t i = 0; i < numExpectedCandidates; ++i)
		{
			const std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generatedCandidate = candidateGenerator.generateNextCandidate();
			ASSERT_TRUE(generatedCandidate.has_value()) << "Failed to generate next candidate, expected to be able to generate " << std::to_string(numExpectedCandidates) << " candidates but only " << std::to_string(i) << " were actually generated";
			actualCandidates.emplace_back(*generatedCandidate);
		}
		ASSERT_FALSE(candidateGenerator.generateNextCandidate());
	}

	static void generateCnfFormulaOrThrow(std::size_t numVariables, const std::initializer_list<std::initializer_list<long>>& formulaClauses, dimacs::ProblemDefinition::ptr& problemDefinition)
	{
		problemDefinition = std::make_shared<dimacs::ProblemDefinition>(numVariables, formulaClauses.size());
		ASSERT_TRUE(problemDefinition);

		std::size_t clauseIdxInFormula = 0;
		for (const auto& clauseLiterals : formulaClauses)
			problemDefinition->addClause(clauseIdxInFormula++, dimacs::ProblemDefinition::Clause(clauseLiterals));
	}
};

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	dimacs::ProblemDefinition::ptr formula;
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(candidateClauseLiterals.size(), { {-1,-2,-3} }, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1 }, { 2 }, { 3 },
		{ 1,2 }, { 1,3 }, { 2,3 },
		{ 1,2,3 }
	});

	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	dimacs::ProblemDefinition::ptr formula;
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(candidateClauseLiterals.size(), { {-1,-2,-3,-4} }, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1 }, { 2 }, { 3 }, { 4 },
		{ 1,2 }, { 1,3 }, { 1,4 }, { 2,3 }, { 2,4 }, { 3,4 },
		{ 1,2,3 }, { 1,2,4 }, { 1,3,4 }, { 2,3,4 },
		{ 1,2,3,4 }
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicsWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	dimacs::ProblemDefinition::ptr formula;
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(candidateClauseLiterals.size(), { {-1,-2,-3,-4,-5} }, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1 }, { 2 }, { 3 }, { 4 }, { 5 },
		{ 1,2 }, { 1,3 }, { 1,4 }, {1,5 }, { 2,3 }, { 2,4 }, { 2,5 }, { 3,4 }, { 3,5 }, { 4,5 },
		{ 1,2,3 }, { 1,2,4 }, {1,2,5 }, { 1,3,4 }, { 1,3,5 }, { 1,4,5 }, { 2,3,4 }, { 2,3,5 }, { 2,4,5 }, { 3,4,5 },
		{ 1,2,3,4 }, { 1,2,3,5 }, { 1,2,4,5 }, { 1,3,4,5 }, { 2,3,4,5 },
		{ 1,2,3,4,5 }
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	constexpr std::size_t numVariablesInFormula = 3;
	// Ordering of variables according to overlap count: 3: 3 > 2: 2 > 1: 1
	dimacs::ProblemDefinition::ptr formula;
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { 2,-3 }
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 1 }, { 2 }, { 3 }, { 1,2 }, { 1,3 }, { 2,3 }, { 1,2,3 } });
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	constexpr std::size_t numVariablesInFormula = 4;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 3: 4 > 2: 3 > 1: 2 > 4: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 4 }, { 1 }, { 2 }, { 3 },
		{ 4,1 }, { 4,2 }, { 4,3 }, { 1,2 }, { 1,3 }, { 2,3 },
		{ 4,1,2 }, { 4,1,3 }, { 4,2,3 }, { 1,2,3 },
		{ 4,1,2,3 }
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 3: 5 > 2: 4 > 1: 3 > 4: 2 > 5: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }, { -3,-2 }, { -1,-4,-5 }
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{5}, {4}, {1}, {2}, {3},
		{5, 4}, {5, 1}, {5, 2}, {5, 3}, {4, 1}, {4, 2}, {4, 3}, {1, 2}, {1, 3}, {2, 3},
		{5, 4, 1}, {5, 4, 2}, {5, 4, 3}, {5, 1, 2}, {5, 1, 3}, {5, 2, 3}, {4, 1, 2}, {4, 1, 3}, {4, 2, 3}, { 1, 2, 3 },
		{5, 4, 1, 2}, {5, 4, 1, 3}, {5, 4, 3, 2}, {5, 1, 2, 3},  { 4, 1, 2, 3 },
		{5, 4, 1, 2, 3}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 3 > 1: 2 > 3: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{-1,-2,-3}, {-1,-2}, {-2,-3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 2 }, { 1 }, { 3 }, { 2,1 }, { 2,3 }, { 1,3 }, { 2,1,3 } });
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,-4 };
	constexpr std::size_t numVariablesInFormula = 4;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > -4: 3 > 1: 2 > 3: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{-1,-2,-3, 4}, {-1,-2, 4}, {-2,-3}, { -2, 4}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 2 }, { -4 }, { 1 }, { 3 },
		{ 2,-4 }, { 2, 1}, { 2,3 }, { -4,1 }, { -4,3 }, { 1,3},
		{ 2,-4,1 }, { 2,-4,3}, { 2,1,3 }, { -4,1,3 },
		{ 2,-4,1,3 }
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 3: 5 > 2: 4 > 1: 3 > 4: 2 > 5: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }, { -3,-2 }, { -1,-4,-5 }
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 3 }, { 2 }, { 1 }, { 4 }, { 5 },
		{ 3,2 }, { 3,1 }, { 3,4 }, { 3,5 }, { 2,1 }, { 2,4 }, { 2,5 }, { 1,4 }, { 1,5 }, { 4,5 },
		{ 3,2,1 }, { 3,2,4 }, { 3,2,5 }, { 3,1,4 }, { 3,1,5 }, { 3,4,5 }, { 2,1,4 }, { 2,1,5 }, { 2,4,5 }, { 1,4,5 },
		{ 3,2,1,4 }, { 3,2,1,5 }, { 3,2,4,5 }, { 3,1,4,5 }, { 2,1,4,5 },
		{ 3,2,1,4,5 }
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{2}, {3}, {1},
		{2, 3}, {2, 1}, {3, 1},
		{2, 3, 1}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1}, {3}, {2},
		{1, 3}, {1, 2}, {3, 2},
		{1, 3, 2}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1}, {2}, {3},
		{1, 2}, {1, 3}, {2, 3},
		{1, 2, 3}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SelectionHeuristicRespectsUserDefinedMaximumBlockingSetSize)
{
	const std::vector<long> candidateClauseLiterals = { 1, 2, 3, -4 };
	constexpr std::size_t numVariablesInFormula = 4;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}, {4, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	const auto& customBlockingSetSizeRestriction = std::make_optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>({ 1,2 });
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup(), customBlockingSetSizeRestriction));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1}, {2}, {3}, {-4},
		{1,2}, {1,3}, {1, -4}, {2,3}, {2,-4}, {3,-4}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SelectionHeuristicRespectsUserDefinedMinimumBlockingSetSize)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}, {4, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	const auto& customBlockingSetSizeRestriction = std::make_optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>({ 3,candidateClauseLiterals.size() });
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup(), customBlockingSetSizeRestriction));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1, 2, 3}, {1, 2, -4}, {1, 3, -4}, {2, 3, -4},
		{1, 2, 3, -4}
	});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SelectionHeuristicRespectsUserDefinedBlockingSetSizeRestrictions)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}, {4, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	const auto& customBlockingSetSizeRestriction = std::make_optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>({ 4,4 });
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup(), customBlockingSetSizeRestriction));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({{1, 2, 3, -4}});
	BlockingSetCandidateCollection actualCandidates;
	ASSERT_NO_FATAL_FAILURE(determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size(), actualCandidates));
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, UserDefinedMaximumBlockingSetSizeLargerThanSetOfClauseLiteralsThrows)
{
	const std::vector<long> candidateClauseLiterals = { 1, -2, 3 };
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { 2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	const auto& customBlockingSetSizeRestriction = std::make_optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>({ 1,candidateClauseLiterals.size() + 1 });
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup(), customBlockingSetSizeRestriction), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, UserDefinedMaximumBlockingSetSizeLargerThanMinimumSizeThrows)
{
	const std::vector<long> candidateClauseLiterals = { 1, -2, 3 };
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { 2, -3}
		}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	const auto& customBlockingSetSizeRestriction = std::make_optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>({ candidateClauseLiterals.size(),1 });
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup(), customBlockingSetSizeRestriction), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeurisiticOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, RandomHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	dimacs::ProblemDefinition::ptr formula;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	ASSERT_NO_FATAL_FAILURE(generateCnfFormulaOrThrow(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	}, formula));

	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingRandomLiteralSelectionHeuristic(4711);
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, formula->getLiteralOccurrenceLookup()), std::invalid_argument);
}