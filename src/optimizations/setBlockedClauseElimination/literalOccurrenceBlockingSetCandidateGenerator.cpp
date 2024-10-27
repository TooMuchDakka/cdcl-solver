#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

#include <random>
#include <string>
#include <unordered_map>

using namespace setBlockedClauseElimination;

std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> LiteralOccurrenceBlockingSetCandidateGenerator::generateNextCandidate()
{
	if (clauseLiterals.empty() || !canGenerateMoreCandidates())
		return std::nullopt;

	bool backTrack = true;
	bool resizedCandidate = false;
	std::size_t lastModifiedIdx = candidateLiteralIndices.size() - 1;
	do
	{
		if (candidateLiteralIndices[lastModifiedIdx] + 1 > getLastIncrementableIndexForPosition(lastModifiedIdx))
		{
			if (lastModifiedIdx)
			{
				--lastModifiedIdx;
				if (requiredWrapAroundBeforeCandidateResize[lastModifiedIdx])
					--requiredWrapAroundBeforeCandidateResize[lastModifiedIdx];
			}

			if (!requiredWrapAroundBeforeCandidateResize[lastModifiedIdx] && !lastModifiedIdx)
			{
				incrementCandidateSize();
				backTrack = false;
				resizedCandidate = true;
			}
		}
		else
		{
			// Update literal for last index in candidate
			if (lastGeneratedCandidate.size() > 1)
				lastGeneratedCandidate.erase(getClauseLiteral(lastModifiedIdx));

			++candidateLiteralIndices[lastModifiedIdx];
			lastGeneratedCandidate.emplace(getClauseLiteral(lastModifiedIdx));
			backTrack = false;
		}
	} while (backTrack);

	// Perform reset of candidate literal indices after backtracking was performed
	for (std::size_t i = lastModifiedIdx + 1; i < candidateLiteralIndices.size() && !resizedCandidate; ++i)
	{
		lastGeneratedCandidate.erase(getClauseLiteral(i));
		candidateLiteralIndices[i] = candidateLiteralIndices[i - 1] + 1;
		lastGeneratedCandidate.emplace(getClauseLiteral(i));

		requiredWrapAroundBeforeCandidateResize[i - 1] = determineRequiredNumberOfWrapAroundsForIndex(candidateLiteralIndices[i - 1]);
	}

	if (!canGenerateMoreCandidates())
		return std::nullopt;

	return lastGeneratedCandidate;
}

// NON PUBLIC FUNCTIONALITY
void LiteralOccurrenceBlockingSetCandidateGenerator::orderLiteralsAccordingToHeuristic(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, bool orderAscendingly)
{
	std::unordered_map<long, std::size_t> overlapCountPerLiteral;
	for (const long literal : clauseLiterals)
	{
		const std::optional<std::size_t>& occurrenceCount = literalOccurrenceLookup.getNumberOfOccurrencesOfLiteral(-literal);
		if (!occurrenceCount.has_value())
			throw std::invalid_argument("Could not determine number of overlapping clauses for literal " + std::to_string(literal));

		overlapCountPerLiteral.emplace(literal, *occurrenceCount);
	}

	if (orderAscendingly)
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
	if (candidateLiteralIndices.size() > clauseLiterals.size())
		return;

	requiredWrapAroundBeforeCandidateResize.emplace_back(0);
	candidateLiteralIndices[0] = 0;

	lastGeneratedCandidate.clear();
	lastGeneratedCandidate.emplace(getClauseLiteral(0));
	for (std::size_t i = 1; i < candidateLiteralIndices.size(); ++i)
	{
		candidateLiteralIndices[i] = candidateLiteralIndices[i - 1] + 1;
		lastGeneratedCandidate.emplace(getClauseLiteral(i));
		requiredWrapAroundBeforeCandidateResize[i-1] = determineRequiredNumberOfWrapAroundsForIndex(candidateLiteralIndices[i - 1]);
	}
}