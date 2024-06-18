#ifndef CLAUSE_LENGTH_BASED_CANDIDATE_SELECTOR_HPP
#define CLAUSE_LENGTH_BASED_CANDIDATE_SELECTOR_HPP
#include <optional>

#include "baseCandidateSelector.hpp"

namespace blockedClauseElimination {
	class ClauseLengthBasedCandidateSelector : public BaseCandidateSelector {
	public:
		enum ClauseLengthSortOrder {
			Ascending,
			Descending
		};

		ClauseLengthBasedCandidateSelector(dimacs::ProblemDefinition::ptr problemDefinition, ClauseLengthSortOrder clauseLengthSortOrder, std::optional<std::size_t> optionalMaximumNumberOfCandidates = std::nullopt)
                    : BaseCandidateSelector(std::move(problemDefinition), optionalMaximumNumberOfCandidates), clauseLengthSortOrder(clauseLengthSortOrder) {}

		[[nodiscard]] std::vector<std::size_t> determineCandidates() override;
	protected:
        ClauseLengthSortOrder clauseLengthSortOrder;

		[[nodiscard]] std::size_t determineNumClauseLiterals(std::size_t idxOfClauseInFormula) const;
	};
}

#endif