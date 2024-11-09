#include "optimizations/utils/clauseCandidateSelector.hpp"

using namespace clauseCandidateSelection;

std::optional<std::size_t> ClauseCandidateSelector::selectNextCandidate()
{
	if (lastChosenCandidateIndexInQueue >= candidateClauseIndexQueue.size())
		return std::nullopt;

	return candidateClauseIndexQueue[lastChosenCandidateIndexInQueue++];
}

// NON-PUBLIC FUNCTIONALITY
void ClauseCandidateSelector::initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition)
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
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				const std::size_t numLClauseOverlaps = determineNumberOfOverlapsBetweenClauses(lClauseIdx, problemDefinition).value_or(SIZE_MAX);
				const std::size_t numRClauseOverlaps = determineNumberOfOverlapsBetweenClauses(rClauseIdx, problemDefinition).value_or(SIZE_MAX);
				if (numLClauseOverlaps == numRClauseOverlaps)
					return lClauseIdx < rClauseIdx;

				return numLClauseOverlaps < numRClauseOverlaps;
			}
		);
		break;
	case CandidateSelectionHeuristic::MaximumClauseOverlap:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineNumberOfOverlapsBetweenClauses(lClauseIdx, problemDefinition).value_or(0)
					> determineNumberOfOverlapsBetweenClauses(rClauseIdx, problemDefinition).value_or(0);
			}
		);
		break;
	case CandidateSelectionHeuristic::MinimumClauseLength:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				const std::size_t lClauseLength = determineLengthOfClause(lClauseIdx, problemDefinition).value_or(SIZE_MAX);
				const std::size_t rClauseLength = determineLengthOfClause(rClauseIdx, problemDefinition).value_or(SIZE_MAX);
				if (lClauseLength == rClauseLength)
					return lClauseIdx < rClauseIdx;

				return lClauseLength < rClauseLength;
			}
		);
		break;
	case CandidateSelectionHeuristic::MaximumClauseLength:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineLengthOfClause(lClauseIdx, problemDefinition).value_or(0)
					> determineLengthOfClause(rClauseIdx, problemDefinition).value_or(0);
			}
		);
		break;
	default:
		throw std::invalid_argument("Clause candidate selector does not support the chosen heuristic: " + std::to_string(candidateSelectionHeuristic));
	}
}

std::optional<std::size_t> ClauseCandidateSelector::determineNumberOfOverlapsBetweenClauses(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition)
{
	const dimacs::ProblemDefinition::Clause* dataOfAccessedClause = problemDefinition.getClauseByIndexInFormula(idxOfClauseInFormula);
	if (!dataOfAccessedClause)
		return std::nullopt;

	std::size_t overlapCount = 0;
	const std::vector<long>& clauseLiterals = dataOfAccessedClause->literals;
	const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup = problemDefinition.getLiteralOccurrenceLookup();
	std::unordered_set<std::size_t> recordedOverlappingClauseIndices;

	for (const long literal : clauseLiterals)
	{
		const std::optional<std::vector<std::size_t>> overlappedClausesForLiteral = literalOccurrenceLookup[-literal];
		if (!overlappedClausesForLiteral.has_value())
			return std::nullopt;

		std::size_t notRecordedOverlapsOfClause = 0;
		for (std::size_t clauseIndex : *overlappedClausesForLiteral)
		{
			notRecordedOverlapsOfClause += recordedOverlappingClauseIndices.count(clauseIndex) == 0;
			recordedOverlappingClauseIndices.emplace(clauseIndex);
		}

		if (SIZE_MAX - overlapCount < notRecordedOverlapsOfClause)
		{
			overlapCount = SIZE_MAX;
			break;
		}
		overlapCount += notRecordedOverlapsOfClause;
	}
	return overlapCount;
}

std::optional<std::size_t> ClauseCandidateSelector::determineLengthOfClause(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition)
{
	if (const std::optional<std::vector<long>>& clauseLiterals = problemDefinition.getClauseLiteralsOmittingAlreadyAssignedOnes(idxOfClauseInFormula); clauseLiterals.has_value())
		return clauseLiterals->size();
	return std::nullopt;
}
