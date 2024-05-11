#include "optimizations/blockedClauseElimination.hpp"

std::vector<std::size_t> blockedClauseElimination::BaseBlockedClauseEliminator::determineCandidatesBasedOnHeuristic() const
{
	return {};
}

std::optional<bool> blockedClauseElimination::BaseBlockedClauseEliminator::isClauseBlocked(std::size_t idxOfClauseToCheck) const
{
	return std::nullopt;
}

// BEGIN PRIVATE FUNCTIONS
std::optional<blockedClauseElimination::BaseBlockedClauseEliminator::SetOfExternalClauseLiteralsReference> blockedClauseElimination::BaseBlockedClauseEliminator::getExternalLiteralsOfClause(std::size_t clauseIdxInFormula) const
{
	return std::nullopt;
}

bool blockedClauseElimination::BaseBlockedClauseEliminator::isClauseBlocked(std::size_t idxOfClauseToCheck, const SetOfExternalClauseLiterals& externalLiteralsOfClause) const
{
	return false;
}

std::unordered_map<std::size_t, blockedClauseElimination::BaseBlockedClauseEliminator::SetOfExternalClauseLiteralsReference> blockedClauseElimination::BaseBlockedClauseEliminator::determineExternalLiteralsPerClauseOfFormula()
{
	return {};
}