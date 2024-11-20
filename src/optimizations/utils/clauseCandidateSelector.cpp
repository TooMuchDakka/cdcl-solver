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
		{
			const auto& overlapCachePerClause = buildOverlapCacheForClauses(problemDefinition, false);
			std::sort(
				candidateClauseIndexQueue.begin(),
				candidateClauseIndexQueue.end(),
				[&problemDefinition, &overlapCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
				{
					return determineOrderingOfElementAccordingToHeuristic(
						lClauseIdx, overlapCachePerClause.at(lClauseIdx),
						rClauseIdx, overlapCachePerClause.at(rClauseIdx),
						false
					);
				}
			);
			break;
		}
		
		case CandidateSelectionHeuristic::MaximumClauseOverlap:
		{
			const auto& overlapCachePerClause = buildOverlapCacheForClauses(problemDefinition, true);
			std::sort(
				candidateClauseIndexQueue.begin(),
				candidateClauseIndexQueue.end(),
				[&problemDefinition, &overlapCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
				{
					return determineOrderingOfElementAccordingToHeuristic(
						lClauseIdx,  overlapCachePerClause.at(lClauseIdx),
						rClauseIdx, overlapCachePerClause.at(rClauseIdx),
						true
					);
				}
			);
			break;
		}
	case CandidateSelectionHeuristic::MinimumClauseLength:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineOrderingOfElementAccordingToHeuristic(
					lClauseIdx, determineLengthOfClause(lClauseIdx, problemDefinition).value_or(SIZE_MAX),
					rClauseIdx, determineLengthOfClause(rClauseIdx, problemDefinition).value_or(SIZE_MAX),
					false
				);
			}
		);
		break;
	case CandidateSelectionHeuristic::MaximumClauseLength:
		std::sort(
			candidateClauseIndexQueue.begin(),
			candidateClauseIndexQueue.end(),
			[&problemDefinition](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
			{
				return determineOrderingOfElementAccordingToHeuristic(
					lClauseIdx, determineLengthOfClause(lClauseIdx, problemDefinition).value_or(0),
					rClauseIdx, determineLengthOfClause(rClauseIdx, problemDefinition).value_or(0),
					true
				);
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
		const std::optional<const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry*>& literalOccurrenceLookupEntry = literalOccurrenceLookup[-literal];
		if (!literalOccurrenceLookupEntry.has_value())
			return std::nullopt;

		if (!*literalOccurrenceLookupEntry)
			continue;

		std::size_t notRecordedOverlapsOfClause = 0;

		const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry& overlappedClausesForLiteral = **literalOccurrenceLookupEntry;
		for (std::size_t clauseIndex : overlappedClausesForLiteral)
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


std::unordered_map<std::size_t, std::size_t> ClauseCandidateSelector::buildOverlapCacheForClauses(const dimacs::ProblemDefinition& problemDefinition, bool usingMaxOverlapAsSelectionHeuristic)
{
	std::unordered_map<std::size_t, std::size_t> overlapCache;
	const std::size_t numClausesInFormula = problemDefinition.getNumClausesAfterOptimizations();
	for (std::size_t i = 0; i < numClausesInFormula; ++i)
		overlapCache.emplace(i, determineNumberOfOverlapsBetweenClauses(i, problemDefinition).value_or(usingMaxOverlapAsSelectionHeuristic ? 0 : SIZE_MAX));
	return overlapCache;
}

