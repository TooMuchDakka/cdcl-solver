#include "optimizations/arrayBasedBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

bool ArrayBasedBlockedClauseEliminator::initializeInternalHelperStructures()
{
	for (std::size_t i = 0; i < problemDefinition->getNumClauses(); ++i)
	{
		const std::optional<dimacs::ProblemDefinition::Clause*> optionalReferencedClause = problemDefinition->getClauseByIndexInFormula(i);
		if (!optionalReferencedClause.has_value())
			return false;

		const dimacs::ProblemDefinition::Clause* referencedClause = *optionalReferencedClause;
		for (const long literal : referencedClause->literals)
		{
			if (!incrementReferenceCountOfLiteral(literal))
				return false;
		}
	}
	return true;
}

std::optional<BaseBlockedClauseEliminator::BlockedClauseSearchResult> ArrayBasedBlockedClauseEliminator::isClauseBlocked(std::size_t idxOfClauseToCheckInFormula)
{
	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToCheckInFormula);
	if (!optionalMatchingClauseForIdx)
		return std::nullopt;

	if (!excludeClauseFromSearchSpace(idxOfClauseToCheckInFormula))
		return std::nullopt;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	const bool isClauseLiteralBlocked = std::any_of(
		matchingClauseForIdx->literals.cbegin(), 
		matchingClauseForIdx->literals.cend(), 
		[&](const long literal){ return getReferenceCountOfLiteral(literal).value_or(0); }
	);

	if (!includeClauseInSearchSpace(idxOfClauseToCheckInFormula))
		return std::nullopt;

	return BlockedClauseSearchResult({isClauseLiteralBlocked, std::nullopt});
}

bool ArrayBasedBlockedClauseEliminator::excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch)
{
	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIgnoreInFurtherSearch);
	if (!optionalMatchingClauseForIdx)
		return false;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	return std::all_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(),
		[&](const long literal){ return decrementReferenceCountOfLiteral(literal); }
	);
}

// BEGIN NON_PUBLIC FUNCTIONALITY
bool ArrayBasedBlockedClauseEliminator::incrementReferenceCountOfLiteral(long literal)
{
	if (!literal)
		return false;

	const std::size_t variableIndex = std::abs(literal);
	if (variableIndex >= variableReferenceCounts.size())
		return false;

	std::size_t& referenceCountsForPolarity = literal < 0 ? variableReferenceCounts.at(variableIndex).numNegativePolarityReferences : variableReferenceCounts.at(variableIndex).numPositivePolarityReferences;
	if (referenceCountsForPolarity == SIZE_MAX)
		return false;

	++referenceCountsForPolarity;
	return true;
}

bool ArrayBasedBlockedClauseEliminator::decrementReferenceCountOfLiteral(long literal)
{
	if (!literal)
		return false;

	const std::size_t variableIndex = std::abs(literal);
	if (variableIndex >= variableReferenceCounts.size())
		return false;

	std::size_t& referenceCountsForPolarity = literal < 0 ? variableReferenceCounts.at(variableIndex).numNegativePolarityReferences : variableReferenceCounts.at(variableIndex).numPositivePolarityReferences;
	if (!referenceCountsForPolarity)
		return false;

	--referenceCountsForPolarity;
	return true;
}

bool ArrayBasedBlockedClauseEliminator::includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch)
{
	const std::optional<dimacs::ProblemDefinition::Clause*> optionalMatchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseToIncludeInFurtherSearch);
	if (!optionalMatchingClauseForIdx)
		return false;

	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = *optionalMatchingClauseForIdx;
	return std::all_of(
		matchingClauseForIdx->literals.cbegin(),
		matchingClauseForIdx->literals.cend(),
		[&](const long literal) { return incrementReferenceCountOfLiteral(literal); }
	);
}

std::optional<std::size_t> ArrayBasedBlockedClauseEliminator::getReferenceCountOfLiteral(long literal) const
{
	if (!literal)
		return std::nullopt;

	const std::size_t variableIndex = std::abs(literal);
	if (variableIndex >= variableReferenceCounts.size())
		return std::nullopt;

	return literal < 0 ? variableReferenceCounts.at(variableIndex).numNegativePolarityReferences : variableReferenceCounts.at(variableIndex).numPositivePolarityReferences;
}