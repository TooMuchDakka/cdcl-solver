#ifndef MINIMUM_CLAUSE_LENGTH_CANDIDATE_SELECTOR_HPP
#define MINIMUM_CLAUSE_LENGTH_CANDIDATE_SELECTOR_HPP
#include <optional>

#include "baseCandidateSelector.hpp"

namespace blockedClauseElimination {
	class MinimumClauseLengthCandidateSelector : public BaseCandidateSelector {
	public:
		MinimumClauseLengthCandidateSelector(dimacs::ProblemDefinition::ptr problemDefinition, std::optional<std::size_t> optionalMaximumNumberOfCandidates = std::nullopt)
			: BaseCandidateSelector(std::move(problemDefinition), optionalMaximumNumberOfCandidates) {}

		[[nodiscard]] std::vector<std::size_t> determineCandidates() override;
	protected:
		[[nodiscard]] std::size_t determineNumClauseLiterals(std::size_t idxOfClauseInFormula) const;
	};
}

#endif