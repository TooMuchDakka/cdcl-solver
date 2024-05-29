#ifndef AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <optimizations/blockedClauseElimination/avlIntervalTree.hpp>
#include "baseBlockedClauseEliminator.hpp"

namespace blockedClauseElimination
{
	class AvlIntervalTreeBlockedClauseEliminator : public BaseBlockedClauseEliminator
	{
	public:
		AvlIntervalTreeBlockedClauseEliminator() = delete;
		explicit AvlIntervalTreeBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseBlockedClauseEliminator(std::move(problemDefinition))
		{
			avlIntervalTree = std::make_unique<avl::AvlIntervalTree>();
		}

		[[nodiscard]] bool initializeInternalHelperStructures() override;
		[[nodiscard]] std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) override;
		[[nodiscard]] bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch) override;
	protected:
		std::unique_ptr<avl::AvlIntervalTree> avlIntervalTree;

		[[nodiscard]] bool includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch) override;
		[[nodiscard]] std::vector<std::size_t> determineSequenceOfClauseIndicesOrderedByToLiteralBounds() const;
		[[nodiscard]] static bool doesResolventContainTautology(const dimacs::ProblemDefinition::Clause* resolventLeftOperand, long resolventLiteral, const dimacs::ProblemDefinition::Clause* resolventRightOperand);
	};
};
#endif