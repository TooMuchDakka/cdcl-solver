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
	const std::unordered_set<std::size_t>& clausesContainingBlockingLiteral = determineIndicesOfOverlappingClausesForLiteral(-potentiallyBlockingLiteral);
	if (clausesContainingBlockingLiteral.empty())
		return false;

	const std::unordered_set<long> literalLookupOfClauseToCheck(clauseLiterals.cbegin(), clauseLiterals.cend());
	return std::all_of(
		clausesContainingBlockingLiteral.cbegin(),
		clausesContainingBlockingLiteral.cend(),
		[&](const std::size_t clauseIndex)
		{
			const dimacs::ProblemDefinition::Clause* referencedClause = problemDefinition->getClauseByIndexInFormula(clauseIndex);
			return referencedClause && checkLiteralBlockedCondition(literalLookupOfClauseToCheck, potentiallyBlockingLiteral, referencedClause->literals);
		});
}

bool BaseBlockedClauseEliminator::checkLiteralBlockedCondition(const std::unordered_set<long>& literalLookupToClauseToCheck, long potentiallyBlockingLiteral, const std::vector<long>& literalOfClauseInResolutionEnvironment)
{
	// A clause C is blocked by a literal l iff for every C' with -l \in C': C \union C' \{-l} is a tautology
	return std::any_of(
		literalOfClauseInResolutionEnvironment.cbegin(),
		literalOfClauseInResolutionEnvironment.cend(),
		[&potentiallyBlockingLiteral, &literalLookupToClauseToCheck](const long literalOfClauseInResolutionEnvironment)
		{
			return literalOfClauseInResolutionEnvironment != -potentiallyBlockingLiteral &&
				literalLookupToClauseToCheck.count(-literalOfClauseInResolutionEnvironment);
		});
}

std::unordered_set<long> BaseBlockedClauseEliminator::constructDifferenceSetBetweenClauseAndPotentiallyBlockingLiteral(const std::vector<long>& clauseLiterals, long potentiallyBlockingLiteral)
{
	std::unordered_set<long> differenceSet(clauseLiterals.cbegin(), clauseLiterals.cend());
	differenceSet.erase(potentiallyBlockingLiteral);
	return differenceSet;
}