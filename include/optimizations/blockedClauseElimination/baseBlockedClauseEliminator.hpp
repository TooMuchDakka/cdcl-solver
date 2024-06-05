#ifndef BASE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <any>
#include <optional>
#include <vector>

#include "dimacs/problemDefinition.hpp"

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator
	{
	public:
		virtual ~BaseBlockedClauseEliminator() = default;
		BaseBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: problemDefinition(std::move(problemDefinition)) {}

		[[nodiscard]] virtual bool initializeInternalHelperStructures() = 0;
		[[nodiscard]] virtual std::vector<std::size_t> determineCandidatesBasedOnHeuristic() const
		{
			std::vector<std::size_t> chooseableCandidateIndices;
			const std::size_t numCandidatesToChoseFrom = problemDefinition->getClauses()->size();
			for (std::size_t i = 0; i < numCandidatesToChoseFrom; ++i)
			{
				chooseableCandidateIndices.emplace_back(i);
			}
			return chooseableCandidateIndices;
		}

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
	};
};
#endif