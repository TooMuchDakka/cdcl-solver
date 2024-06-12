#include "optimizations/blockedClauseElimination/candidateSelection/minimumClauseLengthCandidateSelector.hpp"

using namespace blockedClauseElimination;

std::vector<std::size_t> MinimumClauseLengthCandidateSelector::determineCandidates()
{
	std::vector<std::size_t> perClauseIdxInFormulaContainer = BaseCandidateSelector::determineCandidates();
	std::sort(perClauseIdxInFormulaContainer.begin(),
		perClauseIdxInFormulaContainer.end(),
		[&](const std::size_t idxOfLhsOperandInCompareOperation, const std::size_t idxOfRhsOperandInCompareOperation)
		{
			return determineNumClauseLiterals(idxOfLhsOperandInCompareOperation) < determineNumClauseLiterals(idxOfRhsOperandInCompareOperation);
		});

	const std::size_t numPotentialCandidates = std::min(perClauseIdxInFormulaContainer.size(), optionalMaximumNumberOfCandidates.value_or(problemDefinition->getNumClauses()));
	if (numPotentialCandidates > 1)
		return { perClauseIdxInFormulaContainer.cbegin(), std::next(perClauseIdxInFormulaContainer.cbegin(), numPotentialCandidates - 1) };

	return { 1, perClauseIdxInFormulaContainer.front() };
}

std::size_t MinimumClauseLengthCandidateSelector::determineNumClauseLiterals(std::size_t idxOfClauseInFormula) const
{
	return problemDefinition->getClauseByIndexInFormula(idxOfClauseInFormula).value()->literals.size();
}