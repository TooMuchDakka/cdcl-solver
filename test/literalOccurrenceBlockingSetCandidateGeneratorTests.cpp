#include <gtest/gtest.h>

#include <dimacs/problemDefinition.hpp>
#include <optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp>

using namespace setBlockedClauseElimination;

class LiteralOccurrenceBlockingSetCandidateGeneratorTests : public testing::Test {
public:
	using BlockingSetCandidateCollection = std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate>;
	std::unique_ptr<LiteralOccurrenceBlockingSetCandidateGenerator> candidateSelector;

	void initializeCandidateSelector(
		std::vector<long> clauseLiterals, 
		LiteralOccurrenceBlockingSetCandidateGenerator::CandidateSelectionHeuristic candidateSelectionHeuristic,
		const dimacs::LiteralOccurrenceLookup* literalOccurrenceLookup)
	{
		candidateSelector = std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(std::move(clauseLiterals), candidateSelectionHeuristic, literalOccurrenceLookup);
		EXPECT_TRUE(candidateSelector);
	}

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

	[[nodiscard]] BlockingSetCandidateCollection determineActualCandidatesOrThrow(const std::size_t numExpectedCandidates) const
	{
		BlockingSetCandidateCollection actualCandidates;
		actualCandidates.reserve(numExpectedCandidates);

		for (std::size_t i = 0; i < numExpectedCandidates; ++i)
		{
			const std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generatedCandidate = candidateSelector->generateNextCandidate();
			EXPECT_TRUE(generatedCandidate.has_value());
			actualCandidates.emplace_back(*generatedCandidate);
		}
		EXPECT_FALSE(candidateSelector->generateNextCandidate());
		return actualCandidates;
	}
};

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicOnlyOneCandidatePossible)
{
	const std::vector<long> candidateClauseLiterals = { 1,2 };
	ASSERT_NO_FATAL_FAILURE(initializeCandidateSelector(candidateClauseLiterals, LiteralOccurrenceBlockingSetCandidateGenerator::Sequential, nullptr));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({{ 1,2 } });
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicNoGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3 };
	ASSERT_NO_FATAL_FAILURE(initializeCandidateSelector(candidateClauseLiterals, LiteralOccurrenceBlockingSetCandidateGenerator::Sequential, nullptr));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 2,3 },
		{ 1,2,3 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicWithGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	ASSERT_NO_FATAL_FAILURE(initializeCandidateSelector(candidateClauseLiterals, LiteralOccurrenceBlockingSetCandidateGenerator::Sequential, nullptr));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 1,4 }, { 2,3 }, { 2,4 }, { 3,4 },
		{ 1,2,3 }, { 1,2,4 }, { 1,3,4 }, { 2,3,4 },
		{ 1,2,3,4 }
	});
	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, SequentialHeuristicsWithLargerGapsInSearchSpace)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4,5 };
	ASSERT_NO_FATAL_FAILURE(initializeCandidateSelector(candidateClauseLiterals, LiteralOccurrenceBlockingSetCandidateGenerator::Sequential, nullptr));

	const BlockingSetCandidateCollection& expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 1,4 }, {1,5 }, { 2,3 }, { 2,4 }, { 2,5 }, { 3,4 }, { 3,5 }, { 4,5 },
		{ 1,2,3 }, { 1,2,4 }, {1,2,5 }, { 1,3,4 }, { 1,3,5 }, { 1,4,5 }, { 2,3,4 }, { 2,3,5 }, { 2,4,5 }, { 3,4,5 },
		{ 1,2,3,4 }, { 1,2,3,5 }, { 1,2,4,5 }, { 1,3,4,5 }, { 2,3,4,5 },
		{ 1,2,3,4,5 }
		});

	const BlockingSetCandidateCollection& actualCandidates = determineActualCandidatesOrThrow(expectedCandidates.size());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}