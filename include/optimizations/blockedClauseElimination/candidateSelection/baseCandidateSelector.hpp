#ifndef BASE_CANDIDATE_SELECTOR_HPP
#define BASE_CANDIDATE_SELECTOR_HPP

#include <memory>
#include <vector>

#include "dimacs/problemDefinition.hpp"

namespace blockedClauseElimination {
	class BaseCandidateSelector {
	public:
		using ptr = std::shared_ptr<BaseCandidateSelector>;

		virtual ~BaseCandidateSelector() = default;
		BaseCandidateSelector(dimacs::ProblemDefinition::ptr problemDefinition, std::optional<std::size_t> optionalMaximumNumberOfCandidates = std::nullopt)
			: problemDefinition(std::move(problemDefinition)), optionalMaximumNumberOfCandidates(optionalMaximumNumberOfCandidates) {}

		[[nodiscard]] virtual std::vector<std::size_t> determineCandidates()
		{
			const std::size_t numCandidatesToChoseFrom = std::min(optionalMaximumNumberOfCandidates.value_or(problemDefinition->getNumClauses()), problemDefinition->getNumClauses());
			std::vector<std::size_t> chooseableCandidateIndices;
			chooseableCandidateIndices.reserve(numCandidatesToChoseFrom);

			for (std::size_t i = 0; i < numCandidatesToChoseFrom; ++i)
			{
				if (const std::optional<dimacs::ProblemDefinition::Clause*> matchingClauseForIdx = problemDefinition->getClauseByIndexInFormula(i); matchingClauseForIdx.has_value() && matchingClauseForIdx.value()->literals.size() > 1)
					chooseableCandidateIndices.emplace_back(i);
			}
			chooseableCandidateIndices.shrink_to_fit();
			return chooseableCandidateIndices;
		}
	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
		std::optional<std::size_t> optionalMaximumNumberOfCandidates;
	};
}
#endif