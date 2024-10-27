#include "optimizations/utils/clauseCandidateSelector.hpp"

using namespace clauseCandidateSelection;

std::optional<std::size_t> ClauseCandidateSelector::selectNextCandidate()
{
	if (lastChosenCandidateIndexInQueue >= candidateClauseIndexQueue.size())
		return std::nullopt;

	return candidateClauseIndexQueue[lastChosenCandidateIndexInQueue++];
}

// NON-PUBLIC FUNCTIONALITY
void ClauseCandidateSelector::initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	switch (candidateSelectionHeuristic)
	{
	case CandidateSelectionHeuristic::Sequential:
	case CandidateSelectionHeuristic::Random:
		break;
	case CandidateSelectionHeuristic::MinimumClauseOverlap:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition, &literalOccurrenceLookup](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineNumberOfOverlapsBetweenClauses(lClauseIdx, problemDefinition, literalOccurrenceLookup).value_or(SIZE_MAX)
					< determineNumberOfOverlapsBetweenClauses(rClauseIdx, problemDefinition, literalOccurrenceLookup).value_or(SIZE_MAX);
			}
		);
		break;
	case CandidateSelectionHeuristic::MaximumClauseOverlap:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition, &literalOccurrenceLookup](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineNumberOfOverlapsBetweenClauses(lClauseIdx, problemDefinition, literalOccurrenceLookup).value_or(0)
					< determineNumberOfOverlapsBetweenClauses(rClauseIdx, problemDefinition, literalOccurrenceLookup).value_or(0);
			}
		);
		break;
	default:
		throw std::invalid_argument("Clause candidate selector does not support the chosen heuristic: " + std::to_string(candidateSelectionHeuristic));
	}
}

std::optional<std::size_t> ClauseCandidateSelector::determineNumberOfOverlapsBetweenClauses(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	const std::optional<const dimacs::ProblemDefinition::Clause*> dataOfAccessedClause = problemDefinition.getClauseByIndexInFormula(idxOfClauseInFormula);
	if (!dataOfAccessedClause.has_value())
		return std::nullopt;

	std::size_t overlapCount = 0;
	const std::vector<long>& clauseLiterals = dataOfAccessedClause.value()->literals;

	for (const long literal : clauseLiterals)
	{
		const std::optional<std::size_t> numOverlapsForLiteral = literalOccurrenceLookup.getNumberOfOccurrencesOfLiteral(-literal);
		if (!numOverlapsForLiteral.has_value())
			return std::nullopt;

		if (SIZE_MAX - overlapCount > *numOverlapsForLiteral)
		{
			overlapCount = SIZE_MAX;
			break;
		}
		overlapCount += *numOverlapsForLiteral;
	}
	return overlapCount;
}

