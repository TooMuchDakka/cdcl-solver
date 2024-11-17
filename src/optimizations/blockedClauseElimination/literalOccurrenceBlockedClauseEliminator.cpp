#include "optimizations/blockedClauseElimination/literalOccurrenceBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

std::unordered_set<std::size_t> LiteralOccurrenceBlockedClauseEliminator::determineIndicesOfOverlappingClausesForLiteral(long literal) const
{
	const std::optional<const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry*> overlappingClausesForLiteral = problemDefinition->getLiteralOccurrenceLookup()[literal];
	if (!overlappingClausesForLiteral)
		return {};

	return **overlappingClausesForLiteral;
}
