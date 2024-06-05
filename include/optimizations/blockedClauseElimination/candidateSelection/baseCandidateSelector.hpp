#ifndef BASE_CANDIDATE_SELECTOR_HPP
#define BASE_CANDIDATE_SELECTOR_HPP

#include <memory>
#include <vector>

namespace blockedClauseElimination {
	class BaseCandidateSelector {
	public:
		using ptr = std::shared_ptr<BaseCandidateSelector>;

		virtual ~BaseCandidateSelector() = default;
		BaseCandidateSelector(dimacs::ProblemDefinition::ptr problemDefinition)
			: problemDefinition(std::move(problemDefinition)) {}

		[[nodiscard]] virtual std::vector<std::size_t> determineCandidates()
		{
			std::vector<std::size_t> chooseableCandidateIndices;
			const std::size_t numCandidatesToChoseFrom = problemDefinition->getClauses()->size();
			for (std::size_t i = 0; i < numCandidatesToChoseFrom; ++i)
			{
				chooseableCandidateIndices.emplace_back(i);
			}
			return chooseableCandidateIndices;
		}
	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
	};
}
#endif