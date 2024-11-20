#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

bool AvlIntervalTreeBlockedClauseEliminator::initializeAvlTree() {
	if (avlIntervalTree)
		return false;

	avlIntervalTree = avl::AvlIntervalTree::createFromCnfFormula(problemDefinition);
	return avlIntervalTree != nullptr;
}

bool AvlIntervalTreeBlockedClauseEliminator::doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral) const
{
	return doesEveryClauseInGenericResolutionEnvironmentContainerFullfillLiteralBlockedCondition(clauseToCheck, potentiallyBlockingLiteral, avlIntervalTree->determineIndicesOfClausesContainingLiteral(-potentiallyBlockingLiteral));
}

inline long AvlIntervalTreeBlockedClauseEliminator::determineClauseBoundsDistance(const dimacs::ProblemDefinition::Clause& clause) noexcept
{
	const long lowerBound = clause.getSmallestLiteralOfClause().value_or(0);
	const long upperBound = clause.getLargestLiteralOfClause().value_or(0);
	return upperBound - lowerBound;
}