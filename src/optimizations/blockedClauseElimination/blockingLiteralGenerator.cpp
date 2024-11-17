#include "optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp"

#include <unordered_map>

using namespace blockedClauseElimination;

void BlockingLiteralGenerator::init(std::vector<long> literals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
{
	lastSelectedCandidateIndex = 0;
	candidateLiterals = std::move(literals);
	switch (candidateSelectionHeuristic)
	{
		case Sequential:
			break;
		case Random:
			std::shuffle(candidateLiterals.begin(), candidateLiterals.end(), *optionalRng);
			break;
		case MinClauseOverlap:
			orderLiteralsAccordingToClauseOverlap(candidateLiterals, literalOccurrenceLookup, true);
			break;
		case MaxClauseOverlap:
			orderLiteralsAccordingToClauseOverlap(candidateLiterals, literalOccurrenceLookup, false);
			break;
		default:
			throw std::domain_error("Literal candidate selection heuristic " + std::to_string(candidateSelectionHeuristic) + " is not supported");
	}
}

std::optional<long> BlockingLiteralGenerator::getNextCandiate()
{
	if (lastSelectedCandidateIndex >= candidateLiterals.size())
		return std::nullopt;
	return candidateLiterals.at(lastSelectedCandidateIndex++);
}


// START NON-PUBLIC FUNCTIONALITY
void BlockingLiteralGenerator::orderLiteralsAccordingToClauseOverlap(std::vector<long>& clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, bool orderAscendingly)
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