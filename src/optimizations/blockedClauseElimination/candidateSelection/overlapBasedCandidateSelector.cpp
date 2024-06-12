#include "optimizations/blockedClauseElimination/candidateSelection/overlapBasedCandidateSelector.hpp"

using namespace blockedClauseElimination;

std::vector<std::size_t> OverlapBasedCandidateSelector::determineCandidates()
{
	std::vector<std::size_t> perClauseIdxInFormulaContainer = BaseCandidateSelector::determineCandidates();
	// Do not consider candidates that overlap no other clause
	perClauseIdxInFormulaContainer.erase(std::remove_if(perClauseIdxInFormulaContainer.begin(), perClauseIdxInFormulaContainer.end(), [&](const std::size_t clauseIdx) { return !determineOverlapCountOfClauseInFormula(clauseIdx); }), perClauseIdxInFormulaContainer.end());

	std::sort(perClauseIdxInFormulaContainer.begin(),
		perClauseIdxInFormulaContainer.end(),
		[&](const std::size_t idxOfLhsOperandInCompareOperation, const std::size_t idxOfRhsOperandInCompareOperation)
		{
			return determineOverlapCountOfClauseInFormula(idxOfLhsOperandInCompareOperation) < determineOverlapCountOfClauseInFormula(idxOfRhsOperandInCompareOperation);
		});

	const std::size_t numPotentialCandidates = std::min(perClauseIdxInFormulaContainer.size(), optionalMaximumNumberOfCandidates.value_or(problemDefinition->getNumClauses()));
	if (numPotentialCandidates > 1)
	{
		if (overlapCountSortOrder == MinimumCountsFirst)
			return { perClauseIdxInFormulaContainer.cbegin(), std::next(perClauseIdxInFormulaContainer.cbegin(), numPotentialCandidates - 1) };

		return { perClauseIdxInFormulaContainer.crbegin(), std::next(perClauseIdxInFormulaContainer.crbegin(), numPotentialCandidates - 1) };
	}
	return { 1, perClauseIdxInFormulaContainer.front() };
}

std::vector<OverlapBasedCandidateSelector::VariableCountEntry> OverlapBasedCandidateSelector::determineVariableCountsInFormula() const
{
	std::vector<VariableCountEntry> variableCounts = std::vector(problemDefinition->getNumVariablesInFormula() + 1, VariableCountEntry());
	for (std::size_t i = 0; i < problemDefinition->getNumClauses(); ++i)
	{
		if (const std::optional<dimacs::ProblemDefinition::Clause*> matchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(i); matchingClauseForIdx.has_value())
		{
			for (const long clauseLiteral : matchingClauseForIdx.value()->literals)
			{
				VariableCountEntry& toBeUpdatedVariableCounts = variableCounts.at(std::abs(clauseLiteral));
				toBeUpdatedVariableCounts.numOccurrencesOfPositiveLiteral += clauseLiteral > 0;
				toBeUpdatedVariableCounts.numOccurrencesOfNegativeLiteral += clauseLiteral < 0;
			}
		}
	}
	return variableCounts;
}

std::size_t OverlapBasedCandidateSelector::determineOverlapCountOfClauseInFormula(std::size_t idxOfClauseInFormula) const
{
	const dimacs::ProblemDefinition::Clause* matchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(idxOfClauseInFormula).value();
	std::size_t sumOfOverlappedClauses = 0;
	for (const long clauseLiteral : matchingClauseForIdx->literals)
	{
		if (const std::size_t numOccurrencesOfLiteralInFormula = clauseLiteral < 0 ? variableCountLookup.at(std::abs(clauseLiteral)).numOccurrencesOfNegativeLiteral : variableCountLookup.at(std::abs(clauseLiteral)).numOccurrencesOfPositiveLiteral; numOccurrencesOfLiteralInFormula)
		{
			if (SIZE_MAX - sumOfOverlappedClauses > numOccurrencesOfLiteralInFormula - 1)
				return SIZE_MAX;

			sumOfOverlappedClauses += numOccurrencesOfLiteralInFormula - 1;
		}
	}
	return sumOfOverlappedClauses;
}