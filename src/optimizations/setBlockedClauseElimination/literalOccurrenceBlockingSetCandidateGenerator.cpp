#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

#include <random>
#include <string>
#include <unordered_map>

using namespace setBlockedClauseElimination;

std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> LiteralOccurrenceBlockingSetCandidateGenerator::generateNextCandidate()
{
	if (clauseLiterals.empty() || !canGenerateMoreCandidates())
		return std::nullopt;

	if (getAndResetOneTimeFlagValueIfSet(usingNoneDefaultInitialCandidateSizeOneTimeFlag))
		return lastGeneratedCandidate;

	bool backtracking;
	const std::size_t initialLastModifiedIdx = candidateLiteralIndices.size() - 1;
	std::size_t lastModifiedIdx = initialLastModifiedIdx;
	do
	{
		backtracking = candidateLiteralIndices[lastModifiedIdx] == getLastIncrementableIndexForPosition(lastModifiedIdx);
		if (!backtracking)
			continue;

		if (requiredWrapAroundBeforeCandidateResize[lastModifiedIdx])
			--requiredWrapAroundBeforeCandidateResize[lastModifiedIdx];

		if (!requiredWrapAroundBeforeCandidateResize[lastModifiedIdx])
		{
			lastModifiedIdx -= lastModifiedIdx > 0;
			// Backtracking has reached first index, increase of candidate size is required.
			if (!lastModifiedIdx && !requiredWrapAroundBeforeCandidateResize[lastModifiedIdx])
			{
				// Increment for next candidate size should stop if future candidate is larger than the set of possible entries for a candidate
				if (candidateLiteralIndices.size() == clauseLiterals.size())
				{
					/*
					 * Handling of special case in which requested minimum candiate size matches the size of the set of possible entries for a candidate requires a special flag to be able to distinguish
					 * said special case from the conventional stop condition
					 */
					if (!usingNoneDefaultInitialCandidateSizeOneTimeFlag || getAndResetOneTimeFlagValueIfSet(userDefinedMinimumSizeMatchesMaximumPossibleSizeOneTimeFlag))
						return std::nullopt;
				}
				else
				{
					incrementCandidateSize();
					if (!canGenerateMoreCandidates())
						return std::nullopt;
				}
				return lastGeneratedCandidate;
			}
		}
	} while (backtracking);

	if (!canGenerateMoreCandidates())
		return std::nullopt;

	if (lastModifiedIdx != initialLastModifiedIdx)
	{
		for (std::size_t i = 0; i < candidateLiteralIndices.size() - lastModifiedIdx; ++i)
			lastGeneratedCandidate.erase(getClauseLiteral(lastModifiedIdx + i));

		++candidateLiteralIndices[lastModifiedIdx];
		lastGeneratedCandidate.emplace(getClauseLiteral(lastModifiedIdx));
		requiredWrapAroundBeforeCandidateResize[lastModifiedIdx] = determineRequiredNumberOfWrapAroundsForIndex(lastModifiedIdx);

		for (std::size_t i = lastModifiedIdx + 1; i < candidateLiteralIndices.size(); ++i)
		{
			candidateLiteralIndices[i] = candidateLiteralIndices[i - 1] + 1;
			lastGeneratedCandidate.emplace(getClauseLiteral(i));
			requiredWrapAroundBeforeCandidateResize[i] = determineRequiredNumberOfWrapAroundsForIndex(i);
		}
	}
	else
	{
		for (std::size_t i = lastModifiedIdx; i < candidateLiteralIndices.size(); ++i)
		{
			if (!lastGeneratedCandidate.empty())
				lastGeneratedCandidate.erase(getClauseLiteral(i));

			++candidateLiteralIndices[i];
			lastGeneratedCandidate.emplace(getClauseLiteral(i));
			requiredWrapAroundBeforeCandidateResize[i] = determineRequiredNumberOfWrapAroundsForIndex(i);
		}
	}
	return lastGeneratedCandidate;
}

void LiteralOccurrenceBlockingSetCandidateGenerator::init(const std::vector<long> candidateClauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction)
{
	BaseBlockingSetCandidateGenerator::init(candidateClauseLiterals, literalOccurrenceLookup, optionalCandidateSizeRestriction);

	candidateLiteralIndices.clear();
	lastGeneratedCandidate.clear();
	requiredWrapAroundBeforeCandidateResize.clear();

	assertThatClauseContainsAtleastTwoLiterals(clauseLiterals);
	filterNoneOverlappingLiteralsFromClause(clauseLiterals, literalOccurrenceLookup);

	if (clauseLiterals.empty() || candidateSizeRestriction.minAllowedSize == 0 || candidateSizeRestriction.minAllowedSize > clauseLiterals.size())
	{
		clauseLiterals.clear();
		return;
	}
	if (candidateSizeRestriction.maxAllowedSize > clauseLiterals.size())
		candidateSizeRestriction.maxAllowedSize = clauseLiterals.size();

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

	candidateLiteralIndices.resize(candidateSizeRestriction.minAllowedSize);
	requiredWrapAroundBeforeCandidateResize.resize(candidateSizeRestriction.minAllowedSize);

	candidateLiteralIndices[0] = 0;
	if (candidateSizeRestriction.minAllowedSize > 1)
	{
		setInternalInitialStateAfterCandidateResize();
	}
	else
	{
		requiredWrapAroundBeforeCandidateResize[0] = determineRequiredNumberOfWrapAroundsForIndex(0);
		candidateLiteralIndices[0] = INITIAL_INDEX_VALUE;
	}
	userDefinedMinimumSizeMatchesMaximumPossibleSizeOneTimeFlag = candidateSizeRestriction.minAllowedSize == clauseLiterals.size();
	usingNoneDefaultInitialCandidateSizeOneTimeFlag = optionalCandidateSizeRestriction.has_value() && optionalCandidateSizeRestriction->minAllowedSize > 1;
}

// NON PUBLIC FUNCTIONALITY
bool LiteralOccurrenceBlockingSetCandidateGenerator::handleCandidateGenerationOfSizeOne()
{
	if (candidateLiteralIndices.front() == getLastIncrementableIndexForPosition(0))
		return false;

	if (requiredWrapAroundBeforeCandidateResize.front())
		--requiredWrapAroundBeforeCandidateResize.front();

	if (!lastGeneratedCandidate.empty())
		lastGeneratedCandidate.erase(getClauseLiteral(0));

	++candidateLiteralIndices[0];
	lastGeneratedCandidate.emplace(getClauseLiteral(0));
	return true;
}

void LiteralOccurrenceBlockingSetCandidateGenerator::incrementCandidateSize()
{
	candidateLiteralIndices[0] = 0;
	candidateLiteralIndices.emplace_back(INITIAL_INDEX_VALUE);
	requiredWrapAroundBeforeCandidateResize.emplace_back(0);
	if (candidateLiteralIndices.size() > candidateSizeRestriction.maxAllowedSize)
		return;

	setInternalInitialStateAfterCandidateResize();
}

void LiteralOccurrenceBlockingSetCandidateGenerator::setInternalInitialStateAfterCandidateResize()
{
	lastGeneratedCandidate.clear();
	lastGeneratedCandidate.emplace(getClauseLiteral(0));
	requiredWrapAroundBeforeCandidateResize[0] = determineRequiredNumberOfWrapAroundsForIndex(0);
	for (std::size_t i = 1; i < candidateLiteralIndices.size(); ++i)
	{
		candidateLiteralIndices[i] = candidateLiteralIndices[i - 1] + 1;
		lastGeneratedCandidate.emplace(getClauseLiteral(i));
		requiredWrapAroundBeforeCandidateResize[i] = determineRequiredNumberOfWrapAroundsForIndex(i);
	}
	//candidateLiteralIndices.back() = INITIAL_INDEX_VALUE;
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