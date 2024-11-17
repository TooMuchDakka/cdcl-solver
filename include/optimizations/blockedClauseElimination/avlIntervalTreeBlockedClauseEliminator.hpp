#ifndef AVL_INTERVAL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define AVL_INTERVAL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "optimizations/blockedClauseElimination/baseBlockedClauseEliminator.hpp"
#include "optimizations/utils/avlIntervalTree.hpp"

namespace blockedClauseElimination {
	class AvlIntervalTreeBlockedClauseEliminator : public BaseBlockedClauseEliminator {
	public:
		AvlIntervalTreeBlockedClauseEliminator() = delete;
		AvlIntervalTreeBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseBlockedClauseEliminator(std::move(problemDefinition)) {}

		[[nodiscard]] bool initializeAvlTree();
	protected:
		avl::AvlIntervalTree::ptr avlIntervalTree;

		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const override;
		[[nodiscard]] static long determineClauseBoundsDistance(const dimacs::ProblemDefinition::Clause& clause) noexcept;
	};
}

#endif