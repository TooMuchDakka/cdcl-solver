#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.hpp"
#include "optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp"


using namespace setBlockedClauseElimination;

// Usage of static functions of test fixture class in test: https://google.github.io/googletest/advanced.html#typed-tests
// Initialization of typed instance in test fixture: https://www.codingwiththomas.com/blog/typed-tests-for-interfaces-with-googletest

using SetBlockedClauseEliminatorTypes = testing::Types<LiteralOccurrenceSetBlockedClauseEliminator, AvlIntervalTreeSetBlockedClauseEliminator>;
using BaseSetBlockedClauseEliminatorInstance = std::shared_ptr<BaseSetBlockedClauseEliminator>;

template<typename T>
void initializeSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, BaseSetBlockedClauseEliminatorInstance& setBlockedClauseEliminatorInstance);

template<>
void initializeSetBlockedClauseEliminator<LiteralOccurrenceSetBlockedClauseEliminator>(dimacs::ProblemDefinition::ptr problemDefinition, BaseSetBlockedClauseEliminatorInstance& setBlockedClauseEliminatorInstance)
{
	setBlockedClauseEliminatorInstance = std::make_shared<LiteralOccurrenceSetBlockedClauseEliminator>(problemDefinition);
}

template<>
void initializeSetBlockedClauseEliminator<AvlIntervalTreeSetBlockedClauseEliminator>(dimacs::ProblemDefinition::ptr problemDefinition, BaseSetBlockedClauseEliminatorInstance& setBlockedClauseEliminatorInstance)
{
	auto avlIntervalTreeEliminatorInstance = std::make_shared<AvlIntervalTreeSetBlockedClauseEliminator>(problemDefinition);
	ASSERT_TRUE(avlIntervalTreeEliminatorInstance->initializeAvlTree());
	setBlockedClauseEliminatorInstance = avlIntervalTreeEliminatorInstance;
}

template<typename T>
class SetBlockedClauseEliminatorTests : public testing::Test {
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
		ASSERT_THAT(*actual, testing::UnorderedElementsAreArray(expected));
	}

	static void generateBlockingSetCandidateGeneratorOrThrow(LiteralOccurrenceBlockingSetCandidateGenerator::ptr& blockingSetCandidateGenerator)
	{
		blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
		ASSERT_TRUE(blockingSetCandidateGenerator);
	}

	static void initializeSetBlockedClauseEliminatorInstance(dimacs::ProblemDefinition::ptr problemDefinition, BaseSetBlockedClauseEliminatorInstance& setBlockedClauseEliminatorInstance)
	{
		ASSERT_NO_FATAL_FAILURE(initializeSetBlockedClauseEliminator<T>(std::move(problemDefinition), setBlockedClauseEliminatorInstance));
	}
};

TYPED_TEST_SUITE(SetBlockedClauseEliminatorTests, SetBlockedClauseEliminatorTypes);

TYPED_TEST(SetBlockedClauseEliminatorTests, EmptyResolutionEnvironmentDoesNotCauseClauseToBeSetBlocked)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(3, { {1,2}, {1,2,3}, {1,2} }, problemDefinition));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 0;
	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));
	ASSERT_FALSE(setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator));
}

TYPED_TEST(SetBlockedClauseEliminatorTests, NotAllClausesInResolutionEnvironmentMatchingCausesClauseToBeNotSetBlocked)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(2, { {1,-2}, {1,2}, {-1,-2} }, problemDefinition));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 0;
	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));
	ASSERT_FALSE(setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 1,1 })));
}

TYPED_TEST(SetBlockedClauseEliminatorTests, BlockingSetOfSizeOneCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(2, { {1, -2}, {-1, 2}, {-1, -2} }, problemDefinition));

	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 0;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -2 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator);
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet));
}

TYPED_TEST(SetBlockedClauseEliminatorTests, BlockingSetOfSizeLargerThanOneCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(5, { {1, 2, 3, 4, 5}, {1, -2, 3}, {-1, 2, 3}, {-1, -2, -3, 4, -5 }}, problemDefinition));

	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 3;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -1, -2, -5 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 }));
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet));
}

TYPED_TEST(SetBlockedClauseEliminatorTests, TautologyBetweenDifferenceSetOfProcessedClauseAndClauseOfResolutionEnvironmentCorrectlyDetected)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(3, { {-3, -2, 1}, {-1, 2, -3} }, problemDefinition));

	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 0;
	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { -2 };
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator);
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet));
}

// TODO: Whether we can assume that local variables, variables only used in one polarity in the cnf, where propagated in the cnf during processing effects the detection of blocking sets
//TYPED_TEST(LiteralOccurrenceSetBlockedClauseEliminatorTests, TautologyBetweenClauseOfResolutionEnvironmentAndNotOverlappingPartsOfBlockingSetCorrectlyDetected)
//{
//	dimacs::ProblemDefinition::ptr problemDefinition;
//	ASSERT_NO_FATAL_FAILURE(generateProblemDefinition(4, { {-1, 2, 3, 4}, {1, -2, -3}, {1, -2, 3, -4} }, problemDefinition));
//
//	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
//	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));
//
//	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
//	ASSERT_NO_FATAL_FAILURE(generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));
//
//	constexpr std::size_t clauseToBeChecked = 2;
//	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = { 1, -2, 3 };
//	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 }));
//	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
//}

TYPED_TEST(SetBlockedClauseEliminatorTests, EmptyUnionOfClauseLiteralsAndBlockingSetNotDetectedAsTautology)
{
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(3, { {1, -2, 3}, {-1, 2, -3} }, problemDefinition));

	BaseSetBlockedClauseEliminatorInstance setBlockedClauseEliminatorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeSetBlockedClauseEliminatorInstance(problemDefinition, setBlockedClauseEliminatorInstance));

	LiteralOccurrenceBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateBlockingSetCandidateGeneratorOrThrow(blockingSetCandidateGenerator));

	constexpr std::size_t clauseToBeChecked = 0;
	ASSERT_FALSE(setBlockedClauseEliminatorInstance->determineBlockingSet(clauseToBeChecked, *blockingSetCandidateGenerator, BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ 3,3 })));
}