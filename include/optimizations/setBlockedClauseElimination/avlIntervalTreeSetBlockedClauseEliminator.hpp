#ifndef AVL_INTERVAL_TREE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define AVL_INTERVAL_TREE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "optimizations/utils/avlIntervalTree.hpp"
#include "optimizations/setBlockedClauseElimination/baseSetBlockedClauseEliminator.hpp"

namespace setBlockedClauseElimination {
	class AvlIntervalTreeSetBlockedClauseEliminator : public BaseSetBlockedClauseEliminator {
	public:
		using BaseSetBlockedClauseEliminator::determineBlockingSet;

		AvlIntervalTreeSetBlockedClauseEliminator() = delete;
		explicit AvlIntervalTreeSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseSetBlockedClauseEliminator(std::move(problemDefinition)) {}

		void initializeAvlTree();
	protected:
		avl::AvlIntervalTree::ptr avlIntervalTree;

		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const override;
	};
}

#endif