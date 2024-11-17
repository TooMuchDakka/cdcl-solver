#include "dimacs/problemDefinition.hpp"
#include "gtest/gtest.h"

#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"
#include "optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp"
#include "optimizations/blockedClauseElimination/literalOccurrenceBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

using BlockedClauseEliminatorTypes = testing::Types<LiteralOccurrenceBlockedClauseEliminator, AvlIntervalTreeBlockedClauseEliminator>;
using BlockedClauseEliminatorInstance = std::shared_ptr<BaseBlockedClauseEliminator>;

template<typename T>
void initializeBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, BlockedClauseEliminatorInstance& blockedClauseEliminatorInstance);

template<>
void initializeBlockedClauseEliminator<LiteralOccurrenceBlockedClauseEliminator>(dimacs::ProblemDefinition::ptr problemDefinition, BlockedClauseEliminatorInstance& blockedClauseEliminatorInstance)
{
	blockedClauseEliminatorInstance = std::make_shared<LiteralOccurrenceBlockedClauseEliminator>(std::move(problemDefinition));
}

template<>
void initializeBlockedClauseEliminator<AvlIntervalTreeBlockedClauseEliminator>(dimacs::ProblemDefinition::ptr problemDefinition, BlockedClauseEliminatorInstance& blockedClauseEliminatorInstance)
{
	auto avlIntervalTreeBlockedClauseEliminatorInstance = std::make_shared<AvlIntervalTreeBlockedClauseEliminator>(std::move(problemDefinition));
	ASSERT_TRUE(avlIntervalTreeBlockedClauseEliminatorInstance->initializeAvlTree());
	blockedClauseEliminatorInstance = avlIntervalTreeBlockedClauseEliminatorInstance;
}

template<typename T>
class BlockedClauseEliminatorTests : public testing::Test {
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

	static void initializeBlockedLiteralGeneratorInstance(BlockingLiteralGenerator::ptr& blockingLiteralGenerator, const std::initializer_list<long>& clauseLiterals, const dimacs::ProblemDefinition& problemDefinition)
	{
		blockingLiteralGenerator = BlockingLiteralGenerator::usingSequentialLiteralSelectionHeuristic();
		ASSERT_NO_FATAL_FAILURE(blockingLiteralGenerator->init(clauseLiterals, problemDefinition.getLiteralOccurrenceLookup()));
	}

	static void initializeBlockedClauseEliminatorInstance(dimacs::ProblemDefinition::ptr problemDefinition, BlockedClauseEliminatorInstance& blockedClauseEliminatorInstance)
	{
		ASSERT_NO_FATAL_FAILURE(initializeBlockedClauseEliminator<T>(std::move(problemDefinition), blockedClauseEliminatorInstance));
	}

	static void assertFoundBlockingLiteralMatches(std::size_t clauseIndexInFormula, BlockingLiteralGenerator& blockingLiteralGenerator, const BaseBlockedClauseEliminator& blockingClauseEliminator, const std::optional<long>& expectedBlockingLiteral)
	{
		std::optional<long> foundBlockingLiteral;
		ASSERT_NO_FATAL_FAILURE(foundBlockingLiteral = blockingClauseEliminator.determineBlockingLiteralOfClause(clauseIndexInFormula, blockingLiteralGenerator));

		ASSERT_EQ(expectedBlockingLiteral.has_value(), foundBlockingLiteral.has_value());
		if (expectedBlockingLiteral.has_value())
			ASSERT_EQ(*expectedBlockingLiteral, *foundBlockingLiteral);
	}
};

TYPED_TEST_SUITE(BlockedClauseEliminatorTests, BlockedClauseEliminatorTypes);

TYPED_TEST(BlockedClauseEliminatorTests, CheckingBlockingLiteralForNotExistingClauseDoesNotCrash)
{
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(numVariablesInFormula, {
		{1,2 - 3},
		{-1,2,3},
		{1,2,3}
	}, problemDefinition));

	BlockingLiteralGenerator::ptr blockedLiteralGeneratorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedLiteralGeneratorInstance(blockedLiteralGeneratorInstance, { 1,2,3 }, *problemDefinition));

	BlockedClauseEliminatorInstance blockedClauseEliminator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedClauseEliminatorInstance(problemDefinition, blockedClauseEliminator));

	constexpr std::size_t indexOfClausesCheckedForBlockingLiteral = 3;
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertFoundBlockingLiteralMatches(indexOfClausesCheckedForBlockingLiteral, *blockedLiteralGeneratorInstance, *blockedClauseEliminator, std::nullopt));
}

TYPED_TEST(BlockedClauseEliminatorTests, NotAllClausesInResolutionEnvironmentTautologyFailsBlockedClauseConditionForChosenLiteral)
{
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(numVariablesInFormula, {
		{1,2 - 3},
		{-1,2,3},
		{1,2,3}
		}, problemDefinition));

	BlockingLiteralGenerator::ptr blockedLiteralGeneratorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedLiteralGeneratorInstance(blockedLiteralGeneratorInstance, { 1,2,3 }, *problemDefinition));

	BlockedClauseEliminatorInstance blockedClauseEliminator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedClauseEliminatorInstance(problemDefinition, blockedClauseEliminator));

	constexpr std::size_t indexOfClausesCheckedForBlockingLiteral = 2;
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertFoundBlockingLiteralMatches(indexOfClausesCheckedForBlockingLiteral, *blockedLiteralGeneratorInstance, *blockedClauseEliminator, std::nullopt));
}

TYPED_TEST(BlockedClauseEliminatorTests, AllClausesInResolutionEnvironmentTautologyFullfillsBlockedClauseConditionsForChosenLiteral)
{
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(numVariablesInFormula, {
		{1,2 - 3},
		{-1,2,3},
		{1,-2,3}
	}, problemDefinition));

	BlockingLiteralGenerator::ptr blockedLiteralGeneratorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedLiteralGeneratorInstance(blockedLiteralGeneratorInstance, { 1,-2,3 }, *problemDefinition));

	BlockedClauseEliminatorInstance blockedClauseEliminator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedClauseEliminatorInstance(problemDefinition, blockedClauseEliminator));

	constexpr std::size_t indexOfClausesCheckedForBlockingLiteral = 2;
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertFoundBlockingLiteralMatches(indexOfClausesCheckedForBlockingLiteral, *blockedLiteralGeneratorInstance, *blockedClauseEliminator, -2));
}

TYPED_TEST(BlockedClauseEliminatorTests, AllClausesInResolutionEnvironmentTautologyFullfillsBlockedClauseConditionsForChosenLiteralNotFirstInCandidateSequence)
{
	constexpr std::size_t numVariablesInFormula = 3;
	dimacs::ProblemDefinition::ptr problemDefinition;
	ASSERT_NO_FATAL_FAILURE(TestFixture::generateProblemDefinition(numVariablesInFormula, {
		{1,2 - 3},
		{-1,2,3},
		{1,-2,3}
	}, problemDefinition));

	BlockingLiteralGenerator::ptr blockedLiteralGeneratorInstance;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedLiteralGeneratorInstance(blockedLiteralGeneratorInstance, { -1,2,3 }, *problemDefinition));

	BlockedClauseEliminatorInstance blockedClauseEliminator;
	ASSERT_NO_FATAL_FAILURE(TestFixture::initializeBlockedClauseEliminatorInstance(problemDefinition, blockedClauseEliminator));

	constexpr std::size_t indexOfClausesCheckedForBlockingLiteral = 1;
	ASSERT_NO_FATAL_FAILURE(TestFixture::assertFoundBlockingLiteralMatches(indexOfClausesCheckedForBlockingLiteral, *blockedLiteralGeneratorInstance, *blockedClauseEliminator, 2));
}