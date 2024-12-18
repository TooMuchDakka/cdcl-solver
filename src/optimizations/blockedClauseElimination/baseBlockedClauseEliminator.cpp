#include "optimizations/blockedClauseElimination/baseBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

std::optional<long> BaseBlockedClauseEliminator::determineBlockingLiteralOfClause(std::size_t clauseIndexInFormula, BlockingLiteralGenerator& blockingLiteralGenerator) const
{
	const dimacs::ProblemDefinition::Clause* referencedClause = problemDefinition->getClauseByIndexInFormula(clauseIndexInFormula);
	if (!referencedClause)
		return std::nullopt;

	blockingLiteralGenerator.init(referencedClause->literals, problemDefinition->getLiteralOccurrenceLookup());
	std::optional<long> potentiallyBlockingLiteral = blockingLiteralGenerator.getNextCandiate();

	while (potentiallyBlockingLiteral.has_value())
	{
		if (doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(*referencedClause, *potentiallyBlockingLiteral))
			return *potentiallyBlockingLiteral;

		potentiallyBlockingLiteral = blockingLiteralGenerator.getNextCandiate();
	}
	return std::nullopt;
}

bool BaseBlockedClauseEliminator::checkLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral, const std::vector<long>& literalOfClauseInResolutionEnvironment)
{
	// A clause C is blocked by a literal l iff for every C' with -l \in C': C \union C' \{-l} is a tautology
	return std::any_of(
		literalOfClauseInResolutionEnvironment.cbegin(),
		literalOfClauseInResolutionEnvironment.cend(),
		[&potentiallyBlockingLiteral, &clauseToCheck](const long literalOfClauseInResolutionEnvironment)
		{
			return literalOfClauseInResolutionEnvironment != -potentiallyBlockingLiteral &&
				clauseToCheck.containsLiteral(-literalOfClauseInResolutionEnvironment);
		});
}