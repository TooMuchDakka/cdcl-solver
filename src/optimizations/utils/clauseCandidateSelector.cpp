#include "optimizations/utils/clauseCandidateSelector.hpp"

using namespace clauseCandidateSelection;

std::optional<std::size_t> ClauseCandidateSelector::selectNextCandidate()
{
	if (lastChosenCandidateIndexInQueue >= candidateClauseIndexQueue.size())
		return std::nullopt;

	return candidateClauseIndexQueue[lastChosenCandidateIndexInQueue++];
}

std::size_t ClauseCandidateSelector::getNumCandidates() const
{
	return candidateClauseIndexQueue.size();
}


// NON-PUBLIC FUNCTIONALITY
void ClauseCandidateSelector::initializeCandidateSequence(const dimacs::ProblemDefinition& problemDefinition)
{
	if (optionalClauseLengthRestriction.has_value())
		filterClausesNotMatchingLengthRestriction(problemDefinition, candidateClauseIndexQueue, *optionalClauseLengthRestriction);

	switch (candidateSelectionHeuristic)
	{
		case CandidateSelectionHeuristic::Sequential:
			break;
		case CandidateSelectionHeuristic::Random:
			std::shuffle(candidateClauseIndexQueue.begin(), candidateClauseIndexQueue.end(), *optionalRngEngine);
			break;
		case CandidateSelectionHeuristic::MinimumClauseOverlap:
		{
			const auto& overlapCachePerClause = buildOverlapCacheForClauses(problemDefinition, false);
			std::sort(
				candidateClauseIndexQueue.begin(),
				candidateClauseIndexQueue.end(),
				[&overlapCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
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
				[&overlapCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
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
		{
			const auto& clauseLengthCachePerClause = buildLengthCacheForClauses(problemDefinition, false);
			std::sort(
				candidateClauseIndexQueue.begin(),
				candidateClauseIndexQueue.end(),
				[&clauseLengthCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
				{
					return determineOrderingOfElementAccordingToHeuristic(
						lClauseIdx, clauseLengthCachePerClause.at(lClauseIdx),
						rClauseIdx, clauseLengthCachePerClause.at(rClauseIdx),
						false
					);
				}
			);
			break;
		}
		case CandidateSelectionHeuristic::MaximumClauseLength:
		{
			const auto& clauseLengthCachePerClause = buildLengthCacheForClauses(problemDefinition, true);
			std::sort(
				candidateClauseIndexQueue.begin(),
				candidateClauseIndexQueue.end(),
				[&clauseLengthCachePerClause](const std::size_t lClauseIdx, const std::size_t rClauseIdx)
				{
					return determineOrderingOfElementAccordingToHeuristic(
						lClauseIdx, clauseLengthCachePerClause.at(lClauseIdx),
						rClauseIdx, clauseLengthCachePerClause.at(rClauseIdx),
						true
					);
				}
			);
			break;
		}
	default:
		throw std::invalid_argument("Clause candidate selector does not support the chosen heuristic: " + std::to_string(candidateSelectionHeuristic));
	}
}

std::optional<std::size_t> ClauseCandidateSelector::determineNumberOfOverlapsBetweenClauses(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition)
{
	const dimacs::ProblemDefinition::Clause* dataOfAccessedClause = problemDefinition.getClauseByIndexInFormula(idxOfClauseInFormula);
	if (!dataOfAccessedClause)
		return std::nullopt;

	const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup = problemDefinition.getLiteralOccurrenceLookup();
	std::unordered_set<std::size_t> recordedOverlappingClauseIndices;

	for (const long literal : dataOfAccessedClause->literals)
	{
		const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry* literalOccurrenceLookupEntry = literalOccurrenceLookup[-literal].value_or(nullptr);
		if (!literalOccurrenceLookupEntry)
			continue;

		recordedOverlappingClauseIndices.insert(literalOccurrenceLookupEntry->cbegin(), literalOccurrenceLookupEntry->cend());
	}
	return recordedOverlappingClauseIndices.size();
}

std::optional<std::size_t> ClauseCandidateSelector::determineLengthOfClause(std::size_t idxOfClauseInFormula, const dimacs::ProblemDefinition& problemDefinition)
{
	if (const std::optional<std::vector<long>>& clauseLiterals = problemDefinition.getClauseLiteralsOmittingAlreadyAssignedOnes(idxOfClauseInFormula); clauseLiterals.has_value())
		return clauseLiterals->size();
	return std::nullopt;
}

std::unordered_map<std::size_t, std::size_t> ClauseCandidateSelector::buildOverlapCacheForClauses(const dimacs::ProblemDefinition& problemDefinition, bool usingMaxOverlapAsSelectionHeuristic)
{
	const std::size_t numClausesInFormula = problemDefinition.getNumClausesAfterOptimizations();
	std::unordered_map<std::size_t, std::size_t> overlapCache;
	overlapCache.reserve(numClausesInFormula);

	for (std::size_t i = 0; i < numClausesInFormula; ++i)
		overlapCache.emplace(i, determineNumberOfOverlapsBetweenClauses(i, problemDefinition).value_or(usingMaxOverlapAsSelectionHeuristic ? 0 : SIZE_MAX));
	return overlapCache;
}

std::unordered_map<std::size_t, std::size_t> ClauseCandidateSelector::buildLengthCacheForClauses(const dimacs::ProblemDefinition& problemDefinition, bool usingMaxLengthAsSelectionHeuristic)
{
	const std::size_t numClausesInFormula = problemDefinition.getNumClausesAfterOptimizations();
	std::unordered_map<std::size_t, std::size_t> overlapCache;
	overlapCache.reserve(numClausesInFormula);

	for (std::size_t i = 0; i < numClausesInFormula; ++i)
	{
		const dimacs::ProblemDefinition::Clause* accessedClauseForIndex = problemDefinition.getClauseByIndexInFormula(i);
		const std::size_t clauseLength = accessedClauseForIndex ? accessedClauseForIndex->literals.size() : (usingMaxLengthAsSelectionHeuristic ? 0 : SIZE_MAX);
		overlapCache.emplace(i, clauseLength);
	}
	return overlapCache;
}

void ClauseCandidateSelector::filterClausesNotMatchingLengthRestriction(const dimacs::ProblemDefinition& problemDefinition, std::vector<std::size_t>& clauseIndices, const ClauseLengthRestriction clauseLengthRestriction)
{
	clauseIndices.erase(
		std::remove_if(
			clauseIndices.begin(),
			clauseIndices.end(),
			[&problemDefinition, &clauseLengthRestriction](const std::size_t clauseIndex)
			{
				const dimacs::ProblemDefinition::Clause* accessedClauseForIndex = problemDefinition.getClauseByIndexInFormula(clauseIndex);
				return accessedClauseForIndex ? accessedClauseForIndex->literals.size() > clauseLengthRestriction.maxAllowedClauseLength : false;
			}), clauseIndices.end());
}
