#ifndef AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "avlTree.hpp"
#include "baseBlockedClauseEliminator.hpp"

namespace blockedClauseElimination
{
	class AvlTreeBlockedClauseEliminator : public BaseBlockedClauseEliminator
	{
	public:
		AvlTreeBlockedClauseEliminator() = delete;
		explicit AvlTreeBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseBlockedClauseEliminator(std::move(problemDefinition))
		{
			avlTreeHelperInstance = std::make_unique<avl::AvlTree>();
		}

		[[nodiscard]] bool initializeInternalHelperStructures() override;
		[[nodiscard]] std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) override;
		[[nodiscard]] bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch) override;
	protected:
		avl::AvlTree::ptr avlTreeHelperInstance;

		[[nodiscard]] bool includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch) override;
	};
};
#endif