#ifndef BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include <unordered_set>
#include <optional>
#include <stdexcept>
#include <dimacs/literalOccurrenceLookup.hpp>

namespace setBlockedClauseElimination {
	class BaseBlockingSetCandidateGenerator {
	public:
		using BlockingSetCandidate = std::unordered_set<long>;

		virtual ~BaseBlockingSetCandidateGenerator() = default;

		[[nodiscard]] virtual std::optional<BlockingSetCandidate> generateNextCandidate() = 0;
		virtual void init(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			this->clauseLiterals = std::move(clauseLiterals);
		}
	protected:
		std::vector<long> clauseLiterals;

		static void assertThatClauseContainsAtleastTwoLiterals(const std::vector<long>& clauseLiterals)
		{
			if (clauseLiterals.size() < 2)
				throw std::invalid_argument("Blocking set candidate generator can only be used for clause with at least two literals");
		}
	};
}

#endif