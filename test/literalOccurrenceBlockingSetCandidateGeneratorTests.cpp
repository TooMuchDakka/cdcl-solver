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
		EXPECT_EQ(expected.size(), actual.size());
		return std::all_of(
			expected.cbegin(),
			expected.cend(),
			[&actual](const long literal)
			{
				return actual.count(literal);
			});
	}

	static void assertGeneratedCandidatesAreEqual(const BlockingSetCandidateCollection& expectedCandidates, const BlockingSetCandidateCollection& actualCandidates)
	{
		EXPECT_EQ(expectedCandidates.size(), actualCandidates.size());
		for (std::size_t i = 0; i < expectedCandidates.size(); ++i)
			EXPECT_TRUE(areBlockingSetsEqual(expectedCandidates.at(i), actualCandidates.at(i))) << "Expected candidate: " << stringifyBlockingSet(expectedCandidates.at(i)) << " but was actually: " << stringifyBlockingSet(actualCandidates.at(i));
	}

	[[nodiscard]] static BlockingSetCandidateCollection determineActualCandidatesOrThrow(BaseBlockingSetCandidateGenerator& candidateGenerator, const std::size_t numExpectedCandidates)
	{
		BlockingSetCandidateCollection actualCandidates;
		actualCandidates.reserve(numExpectedCandidates);

		for (std::size_t i = 0; i < numExpectedCandidates; ++i)
		{
			const std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generatedCandidate = candidateGenerator.generateNextCandidate();
			EXPECT_TRUE(generatedCandidate.has_value());
			actualCandidates.emplace_back(*generatedCandidate);
		}
		EXPECT_FALSE(candidateGenerator.generateNextCandidate());
		return actualCandidates;
	}

	[[nodiscard]] static dimacs::ProblemDefinition::ptr generateCnfFormula(const std::size_t numVariables, const std::initializer_list<std::initializer_list<long>>& formulaClauses)
	{
		auto problemDefinition = std::make_shared<dimacs::ProblemDefinition>(numVariables, formulaClauses.size());
		EXPECT_TRUE(problemDefinition);

		std::size_t clauseIdxInFormula = 0;
		for (const auto& clauseLiterals : formulaClauses)
			problemDefinition->addClause(clauseIdxInFormula++, dimacs::ProblemDefinition::Clause(clauseLiterals));

		return problemDefinition;
	}

	[[nodiscard]] static LiteralOccurrenceBlockingSetCandidateGenerator::ptr generateCandidateGenerator()
	{
		LiteralOccurrenceBlockingSetCandidateGenerator::ptr generatorInstance = std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>();
		EXPECT_TRUE(generatorInstance);
		return generatorInstance;
	}
};

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicOnlyOneCandidatePossible)
{
	const std::vector<long> candidateClauseLiterals = { 1,2 };
	const auto formula = generateCnfFormula(candidateClauseLiterals.size(), { {-1,-2} });

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({{ 1,2 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	const auto formula = generateCnfFormula(candidateClauseLiterals.size(), { {-1,-2,-3} });

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 2,3 },
		{ 1,2,3 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	const auto formula = generateCnfFormula(candidateClauseLiterals.size(), { {-1,-2,-3,-4} });

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 1,4 }, { 2,3 }, { 2,4 }, { 3,4 },
		{ 1,2,3 }, { 1,2,4 }, { 1,3,4 }, { 2,3,4 },
		{ 1,2,3,4 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicsWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	const auto formula = generateCnfFormula(candidateClauseLiterals.size(), { {-1,-2,-3,-4,-5} });

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 1,4 }, {1,5 }, { 2,3 }, { 2,4 }, { 2,5 }, { 3,4 }, { 3,5 }, { 4,5 },
		{ 1,2,3 }, { 1,2,4 }, {1,2,5 }, { 1,3,4 }, { 1,3,5 }, { 1,4,5 }, { 2,3,4 }, { 2,3,5 }, { 2,4,5 }, { 3,4,5 },
		{ 1,2,3,4 }, { 1,2,3,5 }, { 1,2,4,5 }, { 1,3,4,5 }, { 2,3,4,5 },
		{ 1,2,3,4,5 }
	});

	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithOnlyOneCandidatePossible)
{
	const std::vector<long> candidateClauseLiterals = { 1,2 };
	constexpr std::size_t numVariablesInFormula = 3;
	// Ordering of variables according to overlap count: 2: 3 > 1: 2 > 3: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{-1,-2,-3}, {-1,-2}, {-2,-3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 1,2 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	constexpr std::size_t numVariablesInFormula = 3;
	// Ordering of variables according to overlap count: 3: 3 > 2: 2 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { 2,-3 }
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 1,2 }, {1,3}, {2,3}, { 1,2,3 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	constexpr std::size_t numVariablesInFormula = 4;
	// Ordering of variables according to overlap count: 3: 4 > 2: 3 > 1: 2 > 4: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ 
		{ 4,1 }, { 4,2 }, { 4,3 }, { 1,2 }, { 1,3 }, { 2,3 },
		{ 4,1,2 }, { 4,1,3 }, { 4,2,3 }, { 1,2,3 },
		{ 4,1,2,3 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 3: 5 > 2: 4 > 1: 3 > 4: 2 > 5: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }, { -3,-2 }, { -1,-4,-5 }
		});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{5, 4}, {5, 1}, {5, 2}, {5, 3}, {4, 1}, {4, 2}, {4, 3}, {1, 2}, {1, 3}, {2, 3},
		{5, 4, 1}, {5, 4, 2}, {5, 4, 3}, {5, 1, 2}, {5, 1, 3}, {5, 2, 3}, {4, 1, 2}, {4, 1, 3}, {4, 2, 3}, { 1, 2, 3 },
		{5, 4, 1, 2}, {5, 4, 1, 3}, {5, 4, 3, 2}, {5, 1, 2, 3},  { 4, 1, 2, 3 },
		{5, 4, 1, 2, 3}
	});

	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithOnlyOneCandidatePossible)
{
	const std::vector<long> candidateClauseLiterals = { 1,2 };
	constexpr std::size_t numVariablesInFormula = 3;
	// Ordering of variables according to overlap count: 2: 3 > 1: 2 > 3: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{-1,-2,-3}, {-1,-2}, {-2,-3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 2,1 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	constexpr std::size_t numVariablesInFormula = 3;
	// Ordering of variables according to overlap count: 2: 3 > 1: 2 > 3: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{-1,-2,-3}, {-1,-2}, {-2,-3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({ { 2,1 }, { 2,3 }, { 1,3 }, { 2,1,3 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,-4 };
	constexpr std::size_t numVariablesInFormula = 4;
	// Ordering of variables according to overlap count: 2: 4 > -4: 3 > 1: 2 > 3: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{-1,-2,-3, 4}, {-1,-2, 4}, {-2,-3}, { -2, 4}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 2,-4 }, { 2, 1}, { 2,3 }, { -4,1 }, { -4,3 }, { 1,3},
		{ 2,-4,1 }, { 2,-4,3}, { 2,1,3 }, { -4,1,3 },
		{ 2,-4,1,3 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 3: 5 > 2: 4 > 1: 3 > 4: 2 > 5: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{ -1,-2,-3 }, { -2,-3 }, { -2,-3 }, { -3,-1,-4 }, { -3,-2 }, { -1,-4,-5 }
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 3,2 }, { 3,1 }, { 3,4 }, { 3,5 }, { 2,1 }, { 2,4 }, { 2,5 }, { 1,4 }, { 1,5 }, { 4,5 },
		{ 3,2,1 }, { 3,2,4 }, { 3,2,5 }, { 3,1,4 }, { 3,1,5 }, { 3,4,5 }, { 2,1,4 }, { 2,1,5 }, { 2,4,5 }, { 1,4,5 },
		{ 3,2,1,4 }, { 3,2,1,5 }, { 3,2,4,5 }, { 3,1,4,5 }, { 2,1,4,5 },
		{ 3,2,1,4,5 }
	});

	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{2, 3}, {2, 1}, {3, 1},
		{2, 3, 1}
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1, 3}, {1, 2}, {3, 2},
		{1, 3, 2}
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicIgnoresLiteralsWithoutOverlap)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,-5, 3,-4 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_NO_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{1, 2}, {1, 3}, {2, 3},
		{1, 2, 3}
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(*candidateSelector, expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeurisiticOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MinClauseOverlapHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, MaxClauseOverlapHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
	});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup), std::invalid_argument);
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, RandomHeuristicOnlyApplicableForClauseWithAtleastTwoLiterals)
{
	const std::vector<long> candidateClauseLiterals = { 1 };
	constexpr std::size_t numVariablesInFormula = 5;
	// Ordering of variables according to overlap count: 2: 4 > 3: 3 > 1: 1
	const auto formula = generateCnfFormula(numVariablesInFormula, {
		{1,-2,-3}, {-1,-2}, {-2,-3}, { -2, -3}
		});

	const dimacs::LiteralOccurrenceLookup literalOccurrenceLookup(*formula);
	const LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateSelector = LiteralOccurrenceBlockingSetCandidateGenerator::usingRandomLiteralSelectionHeuristic(4711);
	ASSERT_THROW(candidateSelector->init(candidateClauseLiterals, literalOccurrenceLookup), std::invalid_argument);
}