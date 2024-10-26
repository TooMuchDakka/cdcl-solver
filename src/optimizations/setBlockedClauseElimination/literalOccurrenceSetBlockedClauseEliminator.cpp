#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>

using namespace setBlockedClauseElimination;

std::optional<std::vector<long>> LiteralOccurrenceSetBlockedClauseEliminator::determineBlockingSet(std::size_t clauseIdxInFormula) const
{
	return std::nullopt;
}

void LiteralOccurrenceSetBlockedClauseEliminator::ignoreSetBlockedClause(std::size_t clauseIdxInFormula)
{
	return;
}


