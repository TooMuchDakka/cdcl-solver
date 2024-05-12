#include "optimizations/blockedClauseElimination.hpp"

#include <algorithm>

using namespace blockedClauseElimination;
std::vector<std::size_t> BaseBlockedClauseEliminator::determineCandidatesBasedOnHeuristic() const
{
	std::vector<std::size_t> chooseableCandidateIndices;
	const std::size_t numCandidatesToChoseFrom = problemDefinition->getClauses()->size();
	for (std::size_t i = 0; i < numCandidatesToChoseFrom; ++i)
	{
		if (canCandidateBeSelectedBasedOnHeuristic(i))
			chooseableCandidateIndices.emplace_back(i);
	}
	return chooseableCandidateIndices;
}

bool BaseBlockedClauseEliminator::canCandidateBeSelectedBasedOnHeuristic(std::size_t) const
{
	return true;
}

std::optional<BaseBlockedClauseEliminator::BlockedClauseSearchResult> BaseBlockedClauseEliminator::isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) const
{
	const std::optional<dimacs::ProblemDefinition::Clause::ptr> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToCheckInFormula);
	if (!optionalMatchingClauseForIdx)
		return std::nullopt;

	const dimacs::ProblemDefinition::Clause::ptr& matchingClauseForIdx = *optionalMatchingClauseForIdx;
	for (const long clauseLiteral : matchingClauseForIdx->literals)
	{
		const BlockedClauseSearchResult searchForBlockingLiteralResult = isClauseLiteralBlocked(idxOfClauseToCheckInFormula, clauseLiteral);
		if (searchForBlockingLiteralResult.isBlocked)
			return searchForBlockingLiteralResult;
	}
	return BlockedClauseSearchResult({ false, 0 });
}

BaseBlockedClauseEliminator::BlockedClauseSearchResult BaseBlockedClauseEliminator::isClauseLiteralBlocked(std::size_t idxOfClauseToCheckInFormula, long literal) const
{
	std::size_t idxOfClauseDefiningBlockingLiteral = 0;
	bool foundMatchingLiteralWithInvertedPolarity = false;
	for (std::size_t i = 0; i < problemDefinition->getClauses()->size() && !foundMatchingLiteralWithInvertedPolarity; ++i)
	{
		if (i == idxOfClauseToCheckInFormula)
			continue;

		const std::optional<dimacs::ProblemDefinition::Clause::ptr> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(i);
		if (!optionalMatchingClauseForIdx)
			continue;

		const dimacs::ProblemDefinition::Clause& matchingClauseForIdx = **optionalMatchingClauseForIdx;
		const dimacs::ProblemDefinition::Clause::LiteralBounds literalBoundsOfClause = matchingClauseForIdx.getLiteralBounds();

		foundMatchingLiteralWithInvertedPolarity = (-literal >= literalBoundsOfClause.smallestLiteral && -literal <= literalBoundsOfClause.largestLiteral) && std::any_of(matchingClauseForIdx.literals.cbegin(), matchingClauseForIdx.literals.cend(), [literal](const long clauseLiteral) { return clauseLiteral == -literal; });
		idxOfClauseDefiningBlockingLiteral = i;
	}
	return BlockedClauseSearchResult({ foundMatchingLiteralWithInvertedPolarity, idxOfClauseDefiningBlockingLiteral });
}