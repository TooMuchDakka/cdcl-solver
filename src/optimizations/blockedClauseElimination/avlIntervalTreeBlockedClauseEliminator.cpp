#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"
#include <algorithm>

using namespace blockedClauseElimination;
bool AvlIntervalTreeBlockedClauseEliminator::initializeInternalHelperStructures()
{
	if (!avlIntervalTree)
		return false;

	for (const std::size_t clauseIndexInFormula : determineSequenceOfClauseIndicesOrderedByToLiteralBounds())
	{
		if (!includeClauseInSearchSpace(clauseIndexInFormula))
			return false;
	}
	return true;
}

std::optional<BaseBlockedClauseEliminator::BlockedClauseSearchResult> AvlIntervalTreeBlockedClauseEliminator::isClauseBlocked(const std::size_t idxOfClauseToCheckInFormula)
{
	if (!avlIntervalTree)
		return std::nullopt;

	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToCheckInFormula);
	if (!optionalMatchingClauseForIdx)
		return std::nullopt;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	// Clauses with only one literal can never be blocked since the resolvent formed with any other clause can never contain a tautology (under the assumption that a clause cannot a tautology itself)
	if (matchingClauseForIdx->literals.size() == 1)
		return BlockedClauseSearchResult({ false, std::nullopt });

	for (const long clauseLiteral : matchingClauseForIdx->literals)
	{
		// Get all potentially overlapping clauses (based on the clause literal bounds)
		for (const std::size_t indexOfClauseWithOverlappingLiteral : avlIntervalTree->getOverlappingIntervalsForLiteral(-clauseLiteral))
		{
			const std::optional<dimacs::ProblemDefinition::Clause*> optionalPotentiallyOverlappingClause = problemDefinition->getClauseByIndexInFormula(indexOfClauseWithOverlappingLiteral);
			if (!optionalPotentiallyOverlappingClause.has_value() || indexOfClauseWithOverlappingLiteral == idxOfClauseToCheckInFormula)
				continue;

			// We can reuse our invariant that a clause with one literal cannot be blocked in a similar way to "short-circuit" our check whether a clause is literal blocked if the clause with one literal is overlapped.
			if (optionalPotentiallyOverlappingClause.value()->literals.size() == 1)
				return BlockedClauseSearchResult({ false, std::nullopt });

			// Check if the resolvent of the clause for which we would like to determine whether it is literal blocked and the clause, for which we have determine that it contains the literal to be looked for with negative polarity, form a tautology
			const dimacs::ProblemDefinition::Clause* clauseOverlappingLiteral = *optionalPotentiallyOverlappingClause;
			if (clauseOverlappingLiteral->containsLiteral(-clauseLiteral) && doesResolventContainTautology(matchingClauseForIdx, clauseLiteral, clauseOverlappingLiteral))
				return BlockedClauseSearchResult({ true, indexOfClauseWithOverlappingLiteral });
		}
	}
	return BlockedClauseSearchResult({ false, std::nullopt });
}

bool AvlIntervalTreeBlockedClauseEliminator::excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch)
{
	if (!avlIntervalTree)
		return true;

	if (const std::optional<const dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIgnoreInFurtherSearch); optionalMatchingClauseForIdx.has_value())
		return avlIntervalTree->removeClause(idxOfClauseToIgnoreInFurtherSearch, optionalMatchingClauseForIdx.value()->getLiteralBounds()) == avl::AvlIntervalTreeNode::Removed;
	return false;
}

// BEGIN NON_PUBLIC FUNCTIONALITY
bool AvlIntervalTreeBlockedClauseEliminator::includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch)
{
	if (!avlIntervalTree)
		return true;

	if (const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIncludeInFurtherSearch); optionalMatchingClauseForIdx.has_value())
		return avlIntervalTree->insertClause(idxOfClauseToIncludeInFurtherSearch, optionalMatchingClauseForIdx.value()->getLiteralBounds());
	return false;
}

std::vector<std::size_t> AvlIntervalTreeBlockedClauseEliminator::determineSequenceOfClauseIndicesOrderedByToLiteralBounds() const
{
	std::vector<std::size_t> sortedClauseIndices(problemDefinition->getNumClauses(), 0);
	for (std::size_t i = 0; i < sortedClauseIndices.size(); ++i)
	{
		if (const std::optional<dimacs::ProblemDefinition::Clause*> matchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(i); matchingClauseForIdx.has_value() && !matchingClauseForIdx.value()->literals.empty())
			sortedClauseIndices.at(i) = i;
	}

	std::sort(
		sortedClauseIndices.begin(),
		sortedClauseIndices.end(),
		[&](const std::size_t idxOfLhsClause, const std::size_t idxOfRhsClause)
		{
			const std::optional<dimacs::ProblemDefinition::Clause*>& matchingClauseForLhsOperand = problemDefinition->getClauseByIndexInFormula(idxOfLhsClause);
			const std::optional<dimacs::ProblemDefinition::Clause*>& matchingClauseForRhsOperand = problemDefinition->getClauseByIndexInFormula(idxOfRhsClause);
			if (!(matchingClauseForLhsOperand.has_value() && matchingClauseForRhsOperand.has_value()))
				return false;
			return matchingClauseForLhsOperand.value()->getLiteralBounds() < matchingClauseForRhsOperand.value()->getLiteralBounds();
		});
	return sortedClauseIndices;
}


bool AvlIntervalTreeBlockedClauseEliminator::doesResolventContainTautology(const dimacs::ProblemDefinition::Clause* resolventLeftOperand, long resolventLiteral, const dimacs::ProblemDefinition::Clause* resolventRightOperand)
{
	return std::any_of(
		resolventLeftOperand->literals.cbegin(),
		resolventLeftOperand->literals.cend(),
		[resolventLiteral, &resolventRightOperand](const long clauseLiteralToCheckForTautology)
		{
			return clauseLiteralToCheckForTautology != resolventLiteral && resolventRightOperand->containsLiteral(-clauseLiteralToCheckForTautology);
		}
	);
}