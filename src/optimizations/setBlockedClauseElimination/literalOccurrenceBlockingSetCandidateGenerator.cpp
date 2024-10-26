#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

#include <random>
#include <string>
#include <unordered_map>

using namespace setBlockedClauseElimination;

std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> LiteralOccurrenceBlockingSetCandidateGenerator::generateNextCandidate()
{
	if (lastGeneratedCandidate.size() > clauseLiterals.size())
		return std::nullopt;

	bool backTrack = true;
	std::size_t lastModifiedIdx = candidateLiteralIndices.size() - 1;
	do
	{
		++candidateLiteralIndices[lastModifiedIdx];
		lastGeneratedCandidate.erase(getClauseLiteral(lastModifiedIdx));
		if (candidateLiteralIndices[lastModifiedIdx] > getLastIncrementableIndexForPosition(lastModifiedIdx))
		{
			if (lastModifiedIdx)
			{
				--lastModifiedIdx;
				--requiredWrapAroundBeforeCandidateResize[lastModifiedIdx];
			}

			if (!requiredWrapAroundBeforeCandidateResize[lastModifiedIdx] && !lastModifiedIdx)
			{
				incrementCandidateSize();
				backTrack = false;
			}
		}
		else
		{
			lastGeneratedCandidate.emplace(getClauseLiteral(lastModifiedIdx));
			backTrack = false;
		}
	} while (backTrack);

	for (std::size_t i = lastModifiedIdx + 1; i < clauseLiterals.size(); ++i)
	{
		candidateLiteralIndices[i] = candidateLiteralIndices[i - 1] + 1;
		lastGeneratedCandidate.emplace(getClauseLiteral(i));

		if (i < clauseLiterals.size() - 1)
			requiredWrapAroundBeforeCandidateResize[i] = determineRequiredNumberOfWrapAroundsForIndex(i);
	}
	return lastGeneratedCandidate;
}

// NON PUBLIC FUNCTIONALITY
void LiteralOccurrenceBlockingSetCandidateGenerator::orderLiteralsAccordingToHeuristic(const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	if (candidateSelectionHeuristic == CandidateSelectionHeuristic::RandomSelection)
	{
		auto rng = std::default_random_engine{};
		std::shuffle( clauseLiterals.begin(), clauseLiterals.end(), rng);
		return;
	}

	if (candidateSelectionHeuristic != CandidateSelectionHeuristic::MaxClauseOverlap && candidateSelectionHeuristic != CandidateSelectionHeuristic::MinClauseOverlap)
		throw std::invalid_argument("Cannot determine order of clause literals for candidate selection heuristic " + std::to_string(candidateSelectionHeuristic));

	std::unordered_map<long, std::size_t> overlapCountPerLiteral;
	for (const long literal : clauseLiterals)
	{
		const std::optional<std::size_t>& occurrenceCount = literalOccurrenceLookup.getNumberOfOccurrencesOfLiteral(literal);
		if (!occurrenceCount.has_value())
			throw std::invalid_argument("Could not determine number of overlapping clauses for literal " + std::to_string(literal));

		overlapCountPerLiteral.emplace(literal, *occurrenceCount);
	}

	if (candidateSelectionHeuristic == CandidateSelectionHeuristic::MinClauseOverlap)
	{
		std::sort(
			clauseLiterals.begin(),
			clauseLiterals.end(),
			[&overlapCountPerLiteral](const long lLiteral, const long rLiteral)
			{
				return overlapCountPerLiteral[lLiteral] < overlapCountPerLiteral[rLiteral];
			});
	}
	else
	{
		std::sort(
			clauseLiterals.begin(),
			clauseLiterals.end(),
			[&overlapCountPerLiteral](const long lLiteral, const long rLiteral)
			{
				return overlapCountPerLiteral[lLiteral] > overlapCountPerLiteral[rLiteral];
			});
	}	
}

void LiteralOccurrenceBlockingSetCandidateGenerator::incrementCandidateSize()
{
	candidateLiteralIndices.emplace_back(0);
	++candidateLiteralIndices[0];

	lastGeneratedCandidate.clear();
	lastGeneratedCandidate.emplace(getClauseLiteral(candidateLiteralIndices[0]));
	requiredWrapAroundBeforeCandidateResize[0] = determineRequiredNumberOfWrapAroundsForIndex(0);
}