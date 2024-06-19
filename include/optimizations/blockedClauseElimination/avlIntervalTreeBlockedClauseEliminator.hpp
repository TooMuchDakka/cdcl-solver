#ifndef AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define AVL_TREE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <optimizations/blockedClauseElimination/intervalTree/avlIntervalTree.hpp>
#include "baseBlockedClauseEliminator.hpp"

namespace blockedClauseElimination
{
	class AvlIntervalTreeBlockedClauseEliminator : public BaseBlockedClauseEliminator
	{
	public:
		AvlIntervalTreeBlockedClauseEliminator() = delete;
		explicit AvlIntervalTreeBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, BaseCandidateSelector::ptr candidateSelector)
			: BaseBlockedClauseEliminator(std::move(problemDefinition), std::move(candidateSelector))
		{
			avlIntervalTree = std::make_unique<avl::AvlIntervalTree>();
		}

		[[nodiscard]] bool initializeInternalHelperStructures() override;
		[[nodiscard]] std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) override;
		[[nodiscard]] bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch) override;
	protected:
		std::unique_ptr<avl::AvlIntervalTree> avlIntervalTree;
		std::unordered_set<long> foundLiteralsBlockingAnyClause;

		[[nodiscard]] bool includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch) override;
		[[nodiscard]] std::vector<std::size_t> determineSequenceOfClauseIndicesOrderedByToLiteralBounds() const;
		[[nodiscard]] bool wasLiteralOfWithNegatedPolarityFoundBlockingClause(long candidateLiteralForWhichNoLiteralWithNegatedPolarityShouldExist) const;
		void recordClauseLiteralAsBlockingAnyClause(long clauseLiteral);
		[[nodiscard]] static bool doesResolventContainTautology(const dimacs::ProblemDefinition::Clause* resolventLeftOperand, long resolventLiteral, const dimacs::ProblemDefinition::Clause* resolventRightOperand);
	};
};
#endif