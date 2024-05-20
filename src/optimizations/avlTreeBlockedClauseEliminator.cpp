#include "optimizations/avlTreeBlockedClauseEliminator.hpp"
#include <algorithm>

using namespace blockedClauseElimination;
bool AvlTreeBlockedClauseEliminator::initializeInternalHelperStructures()
{
	if (!avlTreeHelperInstance)
		return false;

	for (std::size_t i = 0; i < problemDefinition->getNumClauses(); ++i)
	{
		const std::optional<dimacs::ProblemDefinition::Clause*> optionalReferencedClause = problemDefinition->getClauseByIndexInFormula(i);
		if (!optionalReferencedClause.has_value())
			return false;

		const dimacs::ProblemDefinition::Clause* referencedClause = *optionalReferencedClause;
		for (const long literal : referencedClause->literals)
		{
			if (!avlTreeHelperInstance->insert(literal))
				return false;
		}
	}
	return true;
}

std::optional<BaseBlockedClauseEliminator::BlockedClauseSearchResult> AvlTreeBlockedClauseEliminator::isClauseBlocked(const std::size_t idxOfClauseToCheckInFormula)
{
	if (!avlTreeHelperInstance)
		return std::nullopt;

	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToCheckInFormula);
	if (!optionalMatchingClauseForIdx)
		return std::nullopt;

	if (!excludeClauseFromSearchSpace(idxOfClauseToCheckInFormula))
		return std::nullopt;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	const bool isClauseLiteralBlocked = std::any_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(), [&](const long literal) { return avlTreeHelperInstance->find(-literal); }
	);

	if (!includeClauseInSearchSpace(idxOfClauseToCheckInFormula))
		return std::nullopt;

	return BlockedClauseSearchResult({ isClauseLiteralBlocked, std::nullopt });
}

bool AvlTreeBlockedClauseEliminator::excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch)
{
	if (!avlTreeHelperInstance)
		return true;

	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIgnoreInFurtherSearch);
	if (!optionalMatchingClauseForIdx)
		return false;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	return std::all_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(),
		[&](const long literal) { return avlTreeHelperInstance->remove(literal); }
	);
}

// BEGIN NON_PUBLIC FUNCTIONALITY
bool AvlTreeBlockedClauseEliminator::includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch)
{
	if (!avlTreeHelperInstance)
		return true;

	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIncludeInFurtherSearch);
	if (!optionalMatchingClauseForIdx)
		return false;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	return std::all_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(),
		[&](const long literal) { return avlTreeHelperInstance->insert(literal); }
	);
}
