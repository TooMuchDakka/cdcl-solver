#ifndef BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include <unordered_set>
#include <optional>
#include <stdexcept>

namespace setBlockedClauseElimination {
	class BaseBlockingSetCandidateGenerator {
	public:
		using BlockingSetCandidate = std::unordered_set<long>;

		virtual ~BaseBlockingSetCandidateGenerator() = default;
		explicit BaseBlockingSetCandidateGenerator(std::vector<long> clauseLiterals)
		{
			if (clauseLiterals.size() < 2)
				throw std::invalid_argument("Blocking set candidate generator can only be used for clause with at least two literals");
			this->clauseLiterals = std::move(clauseLiterals);
		}

		[[nodiscard]] virtual std::optional<BlockingSetCandidate> generateNextCandidate() = 0;
	protected:
		std::vector<long> clauseLiterals;
	};
}

#endif