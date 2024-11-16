#include <optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.hpp>

using namespace setBlockedClauseElimination;

bool AvlIntervalTreeSetBlockedClauseEliminator::initializeAvlTree() {
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

			const long lClauseMidpoint = lClause->determineLiteralMidpoint();
			const long rClauseMidpoint = rClause->determineLiteralMidpoint();
			if (lClauseMidpoint == rClauseMidpoint)
				return lClauseIndex < rClauseIndex;

			return lClauseMidpoint < rClauseMidpoint;
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

std::unordered_set<std::size_t> AvlIntervalTreeSetBlockedClauseEliminator::determineIndicesOfOverlappingClausesForLiteral(long literal) const {
	return avlIntervalTree->determineIndicesOfClausesContainingLiteral(literal);
}