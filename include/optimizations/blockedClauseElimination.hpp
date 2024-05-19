#ifndef BLOCKED_CLAUSE_ELIMINATION_HPP
#define BLOCKED_CLAUSE_ELIMINATION_HPP

#include <optional>
#include <vector>

#include "dimacs/problemDefinition.hpp"
#include "avlTree.hpp"

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator
	{
	public:
		virtual ~BaseBlockedClauseEliminator() = default;

		explicit BaseBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: problemDefinition(std::move(problemDefinition))
		{
			avlTreeHelperInstance = std::make_unique<avl::AvlTree>();
		}

		[[nodiscard]] virtual bool initializeInternalHelperStructures();
		[[nodiscard]] virtual std::vector<std::size_t> determineCandidatesBasedOnHeuristic() const;
		[[nodiscard]] virtual bool canCandidateBeSelectedBasedOnHeuristic(std::size_t potentialCandidateClauseIdxInFormula) const;

		struct BlockedClauseSearchResult
		{
			bool isBlocked;
			std::optional<std::size_t> optionalIdxOfClauseDefiningBlockingLiteral;
		};
		[[nodiscard]] virtual std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) const;
		[[nodiscard]] virtual bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch);
	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
		avl::AvlTree::ptr avlTreeHelperInstance;
	};
};
#endif