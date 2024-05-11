#include "optimizations/blockedClauseElimination.hpp"
#include "baseBlockedClauseEliminationTestsFixture.hpp"

TEST_F(BaseBlockedClauseEliminationTestsFixture, BaseTest)
{
	const dimacs::ProblemDefinition::Clause::ptr firstClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	firstClause->literals.emplace_back(1);
	firstClause->literals.emplace_back(2);

	const dimacs::ProblemDefinition::Clause::ptr secondClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	secondClause->literals.emplace_back(1);
	secondClause->literals.emplace_back(-2);
	secondClause->literals.emplace_back(3);

	const dimacs::ProblemDefinition::Clause::ptr thirdClause = std::make_shared<dimacs::ProblemDefinition::Clause>();
	thirdClause->literals.emplace_back(-1);
	thirdClause->literals.emplace_back(3);

	const auto& formulaReference = std::make_shared<std::vector<dimacs::ProblemDefinition::Clause::constPtr>>();
	formulaReference->emplace_back(firstClause);
	formulaReference->emplace_back(secondClause);
	formulaReference->emplace_back(thirdClause);

	std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(formulaReference);
}