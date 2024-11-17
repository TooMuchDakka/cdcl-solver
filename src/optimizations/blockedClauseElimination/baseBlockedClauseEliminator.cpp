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
		if (doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(referencedClause->literals, *potentiallyBlockingLiteral))
			return *potentiallyBlockingLiteral;

		potentiallyBlockingLiteral = blockingLiteralGenerator.getNextCandiate();
	}
	return std::nullopt;
}

bool BaseBlockedClauseEliminator::doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(const std::vector<long>& clauseLiterals, long potentiallyBlockingLiteral) const
{
	const std::unordered_set<std::size_t>& clausesContainingBlockingLiteral = determineIndicesOfOverlappingClausesForLiteral(potentiallyBlockingLiteral);
	if (clausesContainingBlockingLiteral.empty())
		return false;

	const std::unordered_set<long>& differenceSetBetweenBlockingLiteralAndClauseToCheck = constructDifferenceSetBetweenClauseAndPotentiallyBlockingLiteral(clauseLiterals, potentiallyBlockingLiteral);

	return std::all_of(
		clausesContainingBlockingLiteral.cbegin(),
		clausesContainingBlockingLiteral.cend(),
		[&](const std::size_t clauseIndex)
		{
			const dimacs::ProblemDefinition::Clause* referencedClause = problemDefinition->getClauseByIndexInFormula(clauseIndex);
			return referencedClause && checkLiteralBlockedCondition(differenceSetBetweenBlockingLiteralAndClauseToCheck, potentiallyBlockingLiteral, referencedClause->literals);
		});
}

bool BaseBlockedClauseEliminator::checkLiteralBlockedCondition(const std::unordered_set<long >& differenceSetBetweenBlockingLiteralAndClauseToCheck, long potentiallyBlockingLiteral, const std::vector<long>& literalOfClauseInResolutionEnvironment)
{
	return std::any_of(
		literalOfClauseInResolutionEnvironment.cbegin(),
		literalOfClauseInResolutionEnvironment.cend(),
		[&potentiallyBlockingLiteral, &differenceSetBetweenBlockingLiteralAndClauseToCheck](const long literalOfClauseInResolutionEnvironment)
		{
			return differenceSetBetweenBlockingLiteralAndClauseToCheck.count(-potentiallyBlockingLiteral)
				|| differenceSetBetweenBlockingLiteralAndClauseToCheck.count(-literalOfClauseInResolutionEnvironment);
		});
}

std::unordered_set<long> BaseBlockedClauseEliminator::constructDifferenceSetBetweenClauseAndPotentiallyBlockingLiteral(const std::vector<long>& clauseLiterals, long potentiallyBlockingLiteral)
{
	std::unordered_set<long> differenceSet(clauseLiterals.cbegin(), clauseLiterals.cend());
	differenceSet.erase(potentiallyBlockingLiteral);
	return differenceSet;
}