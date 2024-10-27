#include <gtest/gtest.h>
#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>

using namespace setBlockedClauseElimination;

class LiteralOccurrenceSetBlockedClauseEliminatorTests : public testing::Test {
public:
	[[nodiscard]] static dimacs::ProblemDefinition::ptr generateProblemDefinition(const std::size_t numVariablesInFormula, const std::initializer_list<std::initializer_list<long>>& clausesOfFormula)
	{
		EXPECT_GT(numVariablesInFormula, 0);
		EXPECT_GT(clausesOfFormula.size(), 0);

		auto problemDefinitionInstance = std::make_shared<dimacs::ProblemDefinition>(numVariablesInFormula, clausesOfFormula.size());
		EXPECT_TRUE(problemDefinitionInstance);

		std::size_t clauseIdx = 0;
		for (const auto literalsOfClause : clausesOfFormula)
			problemDefinitionInstance->addClause(clauseIdx++, dimacs::ProblemDefinition::Clause(std::vector(literalsOfClause.begin(), literalsOfClause.end())));
		return problemDefinitionInstance;
	}

	static void assertThatBlockingSetsMatchOrThrow(const BaseSetBlockedClauseEliminator::FoundBlockingSet& expected, const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet>& actual)
	{
		EXPECT_TRUE(actual.has_value());
		EXPECT_EQ(expected.size(), actual->size());

		const std::unordered_set<long> expectedLiteralsOfBlockingSet(expected.cbegin(), expected.cend());
		const BaseSetBlockedClauseEliminator::FoundBlockingSet& actualLiteralsOfBlockingSet = *actual;
		for (const long literal : actualLiteralsOfBlockingSet)
			EXPECT_TRUE(expectedLiteralsOfBlockingSet.count(literal));
	}
};

TEST_F(LiteralOccurrenceSetBlockedClauseEliminatorTests, BasicTest) {
	const dimacs::ProblemDefinition::ptr problemDefinition = generateProblemDefinition(2, {
		{1, 2}, {-1, 2}, {-2, 1}
	});
	auto setBlockedClauseEliminatorInstance = std::make_unique<LiteralOccurrenceSetBlockedClauseEliminator>(problemDefinition);
	EXPECT_TRUE(setBlockedClauseEliminatorInstance);

	auto blockingSetCandidateGenerator = LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
	EXPECT_TRUE(blockingSetCandidateGenerator);

	const BaseSetBlockedClauseEliminator::FoundBlockingSet expectedBlockingSet = {1,2};
	const std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> actualBlockingSet = setBlockedClauseEliminatorInstance->determineBlockingSet(0, *blockingSetCandidateGenerator);
	assertThatBlockingSetsMatchOrThrow(expectedBlockingSet, actualBlockingSet);
}