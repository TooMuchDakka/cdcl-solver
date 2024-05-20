#ifndef ARRAY_BASED_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define ARRAY_BASED_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "baseBlockedClauseEliminator.hpp"

namespace blockedClauseElimination {
	class ArrayBasedBlockedClauseEliminator : public BaseBlockedClauseEliminator {
	public:
		ArrayBasedBlockedClauseEliminator() = delete;
		explicit ArrayBasedBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseBlockedClauseEliminator(std::move(problemDefinition))
		{
			variableReferenceCounts = std::vector<VariableReferenceCounts>(this->problemDefinition->getNumVariablesInFormula() + 1, VariableReferenceCounts({ 0, 0 }));
		}

		[[nodiscard]] bool initializeInternalHelperStructures() override;
		[[nodiscard]] std::optional<BlockedClauseSearchResult> isClauseBlocked(std::size_t idxOfClauseToCheckInFormula) override;
		[[nodiscard]] bool excludeClauseFromSearchSpace(std::size_t idxOfClauseToIgnoreInFurtherSearch) override;
	protected:
		struct VariableReferenceCounts
		{
			std::size_t numPositivePolarityReferences;
			std::size_t numNegativePolarityReferences;
		};
		std::vector<VariableReferenceCounts> variableReferenceCounts;

		[[maybe_unused]] bool incrementReferenceCountOfLiteral(long literal);
		[[maybe_unused]] bool decrementReferenceCountOfLiteral(long literal);
		[[nodiscard]] bool includeClauseInSearchSpace(std::size_t idxOfClauseToIncludeInFurtherSearch) override;
		[[nodiscard]] std::optional<std::size_t> getReferenceCountOfLiteral(long literal) const;
	};
}


#endif