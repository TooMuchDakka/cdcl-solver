#include <gtest/gtest.h>

#include <dimacs/problemDefinition.hpp>
#include <optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp>

using namespace setBlockedClauseElimination;

class LiteralOccurrenceBlockingSetCandidateGeneratorTests : public testing::Test {
public:
	std::unique_ptr<LiteralOccurrenceBlockingSetCandidateGenerator> candidateSelector;

	void initializeCandidateSelector(
		std::vector<long> clauseLiterals, 
		LiteralOccurrenceBlockingSetCandidateGenerator::CandidateSelectionHeuristic candidateSelectionHeuristic,
		const dimacs::LiteralOccurrenceLookup* literalOccurrenceLookup)
	{
		candidateSelector = std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>(std::move(clauseLiterals), candidateSelectionHeuristic, literalOccurrenceLookup);
		EXPECT_TRUE(candidateSelector);
	}

	[[nodiscard]] static std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> buildExpectedCandidateLookup(const std::initializer_list<std::initializer_list<long>>& expectedCandidates)
	{
		std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> candidateContainer;
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

	static void assertGeneratedCandidatesAreEqual(const std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate>& expectedCandidates, const std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate>& actualCandidates)
	{
		EXPECT_EQ(expectedCandidates.size(), actualCandidates.size());
		for (std::size_t i = 0; i < expectedCandidates.size(); ++i)
			EXPECT_TRUE(areBlockingSetsEqual(expectedCandidates.at(i), actualCandidates.at(i))) << "Expected candidate: " << stringifyBlockingSet(expectedCandidates.at(i)) << " but was actually: " << stringifyBlockingSet(actualCandidates.at(i));
	}
};

TEST_F(LiteralOccurrenceBlockingSetCandidateGeneratorTests, TestOne)
{
	const std::vector<long> candidateClauseLiterals = { 1,2,3,4 };
	ASSERT_NO_FATAL_FAILURE(initializeCandidateSelector(candidateClauseLiterals, LiteralOccurrenceBlockingSetCandidateGenerator::RandomSelection, nullptr));

	const std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> expectedCandidates = buildExpectedCandidateLookup({
		{ 1,2 }, { 1,3 }, { 1,4 }, { 2,3 }, { 2,4 }, { 1,2,3 }, { 1,2,4 }, { 1,3,4 }, { 1,2,3,4 }
	});

	std::vector<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> actualCandidates;
	actualCandidates.reserve(expectedCandidates.size());

	for (std::size_t i = 0; i < expectedCandidates.size(); ++i)
	{
		const std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> generatedCandidate = candidateSelector->generateNextCandidate();
		EXPECT_TRUE(generatedCandidate.has_value());
		actualCandidates.emplace_back(*generatedCandidate);
	}
	EXPECT_FALSE(candidateSelector->generateNextCandidate());
	ASSERT_NO_FATAL_FAILURE(assertGeneratedCandidatesAreEqual(expectedCandidates, actualCandidates));
}