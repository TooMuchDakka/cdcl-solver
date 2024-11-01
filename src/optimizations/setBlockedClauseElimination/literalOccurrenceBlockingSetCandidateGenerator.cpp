#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

#include <random>
#include <string>
#include <unordered_map>

using namespace setBlockedClauseElimination;

std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> LiteralOccurrenceBlockingSetCandidateGenerator::generateNextCandidate()
{
	if (clauseLiterals.empty() || !canGenerateMoreCandidates())
		return std::nullopt;

	bool resizedCandidate = false;
	if (candidateLiteralIndices.size() == 1)
	{
		if (candidateLiteralIndices[0] == (clauseLiterals.size() - 1))
		{
			incrementCandidateSize();
			resizedCandidate = true;
		}
		else
		{
			if (!lastGeneratedCandidate.empty())
			{
				lastGeneratedCandidate.erase(getClauseLiteral(0));
				++candidateLiteralIndices[0];
			}
			lastGeneratedCandidate.emplace(getClauseLiteral(0));
			return lastGeneratedCandidate;
		}
	}

	bool backTrack = true;
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
			if (!resizedCandidate)
			{
				lastGeneratedCandidate.erase(getClauseLiteral(lastModifiedIdx));
				++candidateLiteralIndices[lastModifiedIdx];
			}
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

void LiteralOccurrenceBlockingSetCandidateGenerator::init(const std::vector<long> candidateClauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	candidateLiteralIndices.clear();
	lastGeneratedCandidate.clear();
	requiredWrapAroundBeforeCandidateResize.clear();
	this->clauseLiterals = candidateClauseLiterals;

	assertThatClauseContainsAtleastTwoLiterals(clauseLiterals);
	filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);

	if (clauseLiterals.empty())
		return;

	switch (literalSelectionHeuristic) {
	case LiteralSelectionHeuristic::Random:
		std::shuffle(clauseLiterals.begin(), clauseLiterals.end(), *optionalRng);
		break;
	case LiteralSelectionHeuristic::Sequential:
		break;
	case LiteralSelectionHeuristic::MinimalClauseOverlap:
		orderLiteralsAccordingToHeuristic(clauseLiterals, literalOccurrenceLookup, true);
		break;
	case LiteralSelectionHeuristic::MaximumClauseOverlap:
		orderLiteralsAccordingToHeuristic(clauseLiterals, literalOccurrenceLookup, false);
		break;
	default:
		throw std::domain_error("Literal selection heuristic " + std::to_string(literalSelectionHeuristic) + " is not supported");
	}
	candidateLiteralIndices = { 0 };
	requiredWrapAroundBeforeCandidateResize = { determineRequiredNumberOfWrapAroundsForIndex(0) };
}

// NON PUBLIC FUNCTIONALITY
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
		requiredWrapAroundBeforeCandidateResize[i - 1] = determineRequiredNumberOfWrapAroundsForIndex(candidateLiteralIndices[i - 1]);
	}
}

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

void LiteralOccurrenceBlockingSetCandidateGenerator::filterNoneOverlappingLiteralsFromClause(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	clauseLiterals.erase(
		std::remove_if(
			clauseLiterals.begin(),
			clauseLiterals.end(),
			[&literalOccurrenceLookup](const long literal) { return literalOccurrenceLookup.getNumberOfOccurrencesOfLiteral(-literal) == 0; }),
		clauseLiterals.end()
	);
}