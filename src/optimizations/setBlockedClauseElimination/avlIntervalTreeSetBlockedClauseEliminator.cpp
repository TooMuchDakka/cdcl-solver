#include "optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.hpp"

using namespace setBlockedClauseElimination;

bool AvlIntervalTreeSetBlockedClauseEliminator::initializeAvlTree() {
	if (avlIntervalTree)
		return false;

	avlIntervalTree = avl::AvlIntervalTree::createFromCnfFormula(problemDefinition);
	return avlIntervalTree != nullptr;
}

std::unordered_set<std::size_t> AvlIntervalTreeSetBlockedClauseEliminator::determineIndicesOfOverlappingClausesForLiteral(long literal) const {
	return avlIntervalTree->determineIndicesOfClausesContainingLiteral(literal);
}

inline long AvlIntervalTreeSetBlockedClauseEliminator::determineClauseBoundsDistance(const dimacs::ProblemDefinition::Clause& clause) noexcept
{
	const long lowerBound = clause.getSmallestLiteralOfClause().value_or(0);
	const long upperBound = clause.getLargestLiteralOfClause().value_or(0);
	return upperBound - lowerBound;
}
