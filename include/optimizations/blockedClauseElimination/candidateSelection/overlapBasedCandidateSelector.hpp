#ifndef MINIMUM_OVERLAP_CANDIDATE_SELECTOR_HPP
#define MINIMUM_OVERLAP_CANDIDATE_SELECTOR_HPP
#include "baseCandidateSelector.hpp"

namespace blockedClauseElimination {
	class OverlapBasedCandidateSelector : public BaseCandidateSelector {
	public:
		enum OverlapCountSortOrder
		{
			MinimumCountsFirst,
			MaximumCountsFirst
		};

		OverlapBasedCandidateSelector(dimacs::ProblemDefinition::ptr problemDefinition, std::optional<std::size_t> optionalMaximumNumberOfCandidates = std::nullopt, OverlapCountSortOrder overlapCountSortOrder = OverlapCountSortOrder::MinimumCountsFirst)
			: BaseCandidateSelector(std::move(problemDefinition), optionalMaximumNumberOfCandidates), overlapCountSortOrder(overlapCountSortOrder), variableCountLookup(determineVariableCountsInFormula()) {}

		[[nodiscard]] std::vector<std::size_t> determineCandidates() override;
	protected:
		struct VariableCountEntry
		{
			std::size_t numOccurrencesOfPositiveLiteral;
			std::size_t numOccurrencesOfNegativeLiteral;
		};
		OverlapCountSortOrder overlapCountSortOrder;
		std::vector<VariableCountEntry> variableCountLookup;

		[[nodiscard]] std::vector<VariableCountEntry> determineVariableCountsInFormula() const;
		[[nodiscard]] std::size_t determineOverlapCountOfClauseInFormula(std::size_t idxOfClauseInFormula) const;
	};
}
#endif