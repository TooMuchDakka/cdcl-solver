#ifndef BASE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <any>
#include <optional>
#include <vector>

#include "dimacs/problemDefinition.hpp"
#include "candidateSelection/baseCandidateSelector.hpp"

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator
	{
	public:
		virtual ~BaseBlockedClauseEliminator() = default;
		BaseBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, BaseCandidateSelector::ptr candidateSelector)
			: problemDefinition(std::move(problemDefinition)), candidateSelector(std::move(candidateSelector)) {}

		[[nodiscard]] virtual bool initializeInternalHelperStructures() = 0;
		[[nodiscard]] virtual std::vector<std::size_t> determineCandidatesBasedOnHeuristic() const { return candidateSelector->determineCandidates(); }

		struct BlockedClauseSearchResult
		{
			bool isBlocked;
			std::optional<std::size_t> optionalIdxOfClauseDefiningBlockingLiteral;
		};
		[[nodiscard]] virtual std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) = 0;
		[[nodiscard]] virtual bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch) = 0;
		[[nodiscard]] virtual bool includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch) = 0;
	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
		BaseCandidateSelector::ptr candidateSelector;
	};
};
#endif