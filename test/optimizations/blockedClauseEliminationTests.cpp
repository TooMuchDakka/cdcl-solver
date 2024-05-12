#include "optimizations/blockedClauseElimination.hpp"
#include "baseBlockedClauseEliminationTestsFixture.hpp"

TEST_F(BaseBlockedClauseEliminationTestsFixture, FormulaWithOnlyOneClause)
{
	constexpr std::size_t idxOfClauseInFormula = 0;
	const dimacs::ProblemDefinition::Clause::ptr clauseOfFormula = std::make_shared<dimacs::ProblemDefinition::Clause>();
	clauseOfFormula->literals.emplace_back(1);
	clauseOfFormula->literals.emplace_back(2);

	const auto& formulaReference = std::make_shared<std::vector<dimacs::ProblemDefinition::Clause::ptr>>();
	formulaReference->emplace_back(clauseOfFormula);

	const auto& problemDefinition = std::make_shared<dimacs::ProblemDefinition>(2, 1);
	problemDefinition->addClause(idxOfClauseInFormula, clauseOfFormula);

	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(problemDefinition);
	EXPECT_FALSE(blockedClauseEliminator->isClauseBlocked(idxOfClauseInFormula).value_or(false));
}

TEST_F(BaseBlockedClauseEliminationTestsFixture, FormulaWithAllClausesBlocked)
{
	constexpr std::size_t idxOfFirstClauseInFormula = 0;
	const dimacs::ProblemDefinition::Clause::ptr firstClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	firstClause->literals.emplace_back(1);
	firstClause->literals.emplace_back(2);

	constexpr std::size_t idxOfSecondClauseInFormula = 1;
	const dimacs::ProblemDefinition::Clause::ptr secondClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	secondClause->literals.emplace_back(1);
	secondClause->literals.emplace_back(-2);
	secondClause->literals.emplace_back(3);

	constexpr std::size_t idxOfThirdClauseInFormula = 2;
	const dimacs::ProblemDefinition::Clause::ptr thirdClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	thirdClause->literals.emplace_back(-1);
	thirdClause->literals.emplace_back(3);

	const auto& formulaReference = std::make_shared<std::vector<dimacs::ProblemDefinition::Clause::ptr>>();
	formulaReference->emplace_back(firstClause);
	formulaReference->emplace_back(secondClause);
	formulaReference->emplace_back(thirdClause);

	const auto& problemDefinition = std::make_shared<dimacs::ProblemDefinition>(3, 3);
	problemDefinition->addClause(idxOfFirstClauseInFormula, firstClause);
	problemDefinition->addClause(idxOfSecondClauseInFormula, secondClause);
	problemDefinition->addClause(idxOfThirdClauseInFormula, thirdClause);

	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(problemDefinition);

	EXPECT_TRUE(blockedClauseEliminator->isClauseBlocked(idxOfFirstClauseInFormula).value_or(false));
	EXPECT_TRUE(blockedClauseEliminator->isClauseBlocked(idxOfSecondClauseInFormula).value_or(false));
	EXPECT_TRUE(blockedClauseEliminator->isClauseBlocked(idxOfThirdClauseInFormula).value_or(false));
}

TEST_F(BaseBlockedClauseEliminationTestsFixture, FormulaWithMultipleClausesWithSharedVariablesOfSamePolarity)
{
	constexpr std::size_t idxOfFirstClauseInFormula = 0;
	const auto firstClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	firstClause->literals.emplace_back(1);
	firstClause->literals.emplace_back(2);

	constexpr std::size_t idxOfSecondClauseInFormula = 1;
	const auto secondClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	secondClause->literals.emplace_back(3);
	secondClause->literals.emplace_back(1);

	constexpr std::size_t idxOfThirdClauseInFormula = 2;
	const auto thirdClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	thirdClause->literals.emplace_back(2);
	thirdClause->literals.emplace_back(4);

	const auto& problemDefinition = std::make_shared<dimacs::ProblemDefinition>(4, 3);
	problemDefinition->addClause(idxOfFirstClauseInFormula, firstClause);
	problemDefinition->addClause(idxOfSecondClauseInFormula, secondClause);
	problemDefinition->addClause(idxOfThirdClauseInFormula, thirdClause);

	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(problemDefinition);

	EXPECT_FALSE(blockedClauseEliminator->isClauseBlocked(idxOfFirstClauseInFormula).value_or(false));
	EXPECT_FALSE(blockedClauseEliminator->isClauseBlocked(idxOfSecondClauseInFormula).value_or(false));
	EXPECT_FALSE(blockedClauseEliminator->isClauseBlocked(idxOfThirdClauseInFormula).value_or(false));
}