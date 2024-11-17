#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"

using namespace blockedClauseElimination;

bool AvlIntervalTreeBlockedClauseEliminator::initializeAvlTree() {
	if (!avlIntervalTree)
		avlIntervalTree = std::make_shared<avl::AvlIntervalTree>(problemDefinition);
	else
		return false;

	const std::size_t numClausesInFormula = problemDefinition->getNumClausesAfterOptimizations();
	std::vector<std::size_t> clauseIndicesOrderedByClauseMidpoint(numClausesInFormula, 0);
	for (std::size_t i = 0; i < numClausesInFormula; ++i)
		clauseIndicesOrderedByClauseMidpoint[i] = i;

	std::sort(
		clauseIndicesOrderedByClauseMidpoint.begin(),
		clauseIndicesOrderedByClauseMidpoint.end(),
		[&](const std::size_t lClauseIndex, const std::size_t rClauseIndex)
		{
			const dimacs::ProblemDefinition::Clause* lClause = problemDefinition->getClauseByIndexInFormula(lClauseIndex);
			const dimacs::ProblemDefinition::Clause* rClause = problemDefinition->getClauseByIndexInFormula(rClauseIndex);

			const long lClauseBoundsDistance = determineClauseBoundsDistance(*lClause);
			const long rClauseBoundsDistance = determineClauseBoundsDistance(*rClause);

			if (lClauseBoundsDistance == rClauseBoundsDistance)
				return lClauseIndex < rClauseIndex;
			return lClauseBoundsDistance < rClauseBoundsDistance;
		});

	return std::all_of(
		clauseIndicesOrderedByClauseMidpoint.cbegin(),
		clauseIndicesOrderedByClauseMidpoint.cend(),
		[&](const std::size_t clauseIndex)
		{
			const std::optional<const dimacs::ProblemDefinition::Clause*> toBeInsertedClause = problemDefinition->getClauseByIndexInFormula(clauseIndex);
			return toBeInsertedClause.has_value() && avlIntervalTree->insertClause(clauseIndex, **toBeInsertedClause);
		});
}

std::unordered_set<std::size_t> AvlIntervalTreeBlockedClauseEliminator::determineIndicesOfOverlappingClausesForLiteral(long literal) const {
	return avlIntervalTree->determineIndicesOfClausesContainingLiteral(literal);
}

inline long AvlIntervalTreeBlockedClauseEliminator::determineClauseBoundsDistance(const dimacs::ProblemDefinition::Clause& clause) noexcept
{
	const long lowerBound = clause.getSmallestLiteralOfClause().value_or(0);
	const long upperBound = clause.getLargestLiteralOfClause().value_or(0);
	return upperBound - lowerBound;
}