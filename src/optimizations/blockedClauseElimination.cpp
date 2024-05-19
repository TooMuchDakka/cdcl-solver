#include "optimizations/blockedClauseElimination.hpp"

#include <algorithm>

using namespace blockedClauseElimination;
bool BaseBlockedClauseEliminator::initializeInternalHelperStructures()
{
	if (!avlTreeHelperInstance)
		return false;

	const std::shared_ptr<std::vector<dimacs::ProblemDefinition::Clause::ptr>> formulaClauses = problemDefinition->getClauses();
	if (!formulaClauses)
		return false;

	for (const dimacs::ProblemDefinition::Clause::ptr& formulaClause : *formulaClauses)
	{
		for (const long clauseLiteral : formulaClause->literals)
		{
			if (!avlTreeHelperInstance->insert(clauseLiteral))
				return false;
		}
	}
	return true;
}


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

std::optional<BaseBlockedClauseEliminator::BlockedClauseSearchResult> BaseBlockedClauseEliminator::isClauseBlocked(const std::size_t idxOfClauseToCheckInFormula) const
{
	if (!avlTreeHelperInstance)
		return std::nullopt;

	const std::optional<dimacs::ProblemDefinition::Clause::ptr> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToCheckInFormula);
	if (!optionalMatchingClauseForIdx)
		return std::nullopt;

	const dimacs::ProblemDefinition::Clause::ptr& matchingClauseForIdx = *optionalMatchingClauseForIdx;
	/*for (const long clauseLiteral : matchingClauseForIdx->literals)
	{
		if (!avlTreeHelperInstance->remove(clauseLiteral))
			return std::nullopt;
	}*/

	const bool isClauseLiteralBlocked = std::any_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(), [&](const long literal) { return avlTreeHelperInstance->find(-literal); }
	);

	/*for (const long clauseLiteral : matchingClauseForIdx->literals)
	{
		if (!avlTreeHelperInstance->insert(clauseLiteral))
			return std::nullopt;
	}*/
	return BlockedClauseSearchResult({ isClauseLiteralBlocked, std::nullopt });
}

bool BaseBlockedClauseEliminator::excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch)
{
	if (!avlTreeHelperInstance)
		return true;

	const std::optional<dimacs::ProblemDefinition::Clause::ptr> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIgnoreInFurtherSearch);
	if (!optionalMatchingClauseForIdx)
		return false;

	const dimacs::ProblemDefinition::Clause::ptr& matchingClauseForIdx = *optionalMatchingClauseForIdx;
	return std::all_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(),
		[&](const long literal)
		{
			return avlTreeHelperInstance->remove(literal);
		});
}