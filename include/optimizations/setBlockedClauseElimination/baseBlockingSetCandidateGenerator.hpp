#ifndef BLOCKING_SET_CANDIDATE_GENERATOR_HPP
#define BLOCKING_SET_CANDIDATE_GENERATOR_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <dimacs/literalOccurrenceLookup.hpp>

namespace setBlockedClauseElimination {
	class BaseBlockingSetCandidateGenerator {
	public:
		struct CandidateSizeRestriction
		{
			std::size_t minAllowedSize;
			std::size_t maxAllowedSize;
		};
		using BlockingSetCandidate = std::unordered_set<long>;
		using ptr = std::unique_ptr<BaseBlockingSetCandidateGenerator>;

		virtual ~BaseBlockingSetCandidateGenerator() = default;
		BaseBlockingSetCandidateGenerator()
			: candidateSizeRestriction({0,0}) {}

		[[nodiscard]] virtual std::optional<BlockingSetCandidate> generateNextCandidate() = 0;
		virtual void init(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup)
		{
			init(std::move(clauseLiterals), literalOccurrenceLookup,std::nullopt);
		}

		virtual void init(std::vector<long> clauseLiterals, const dimacs::LiteralOccurrenceLookup& literalOccurrenceLookup, const std::optional<CandidateSizeRestriction>& optionalCandidateSizeRestriction)
		{
			this->clauseLiterals = std::move(clauseLiterals);
			if (optionalCandidateSizeRestriction.has_value())
			{
				if (optionalCandidateSizeRestriction->maxAllowedSize < optionalCandidateSizeRestriction->minAllowedSize)
					throw std::invalid_argument("Maximum candidate size of " + std::to_string(optionalCandidateSizeRestriction->maxAllowedSize) + " cannot be smaller than the configured minimum size " + std::to_string(optionalCandidateSizeRestriction->minAllowedSize));

				candidateSizeRestriction = *optionalCandidateSizeRestriction;
				if (candidateSizeRestriction.maxAllowedSize > this->clauseLiterals.size())
					candidateSizeRestriction.maxAllowedSize = this->clauseLiterals.size();
			}
			else
				candidateSizeRestriction = CandidateSizeRestriction({ 1, this->clauseLiterals.size() });
		}
	protected:
		std::vector<long> clauseLiterals;
		CandidateSizeRestriction candidateSizeRestriction;

		/*
		 * Local variable propgation during processing of cnf could prevent the detection of blocking set (see the following cnf)
		 * p cnf 4 3
		 * -1 2 3 4 0
		 * -1 -2 3 0
		 * 1 -2 3 4 0
		 *
		 * Due to our assumption that literals without overlap are not considered as candidates for the blocking set, the blocking set {1, -2, 3} would not be detected.
		 *
		 */
	};
}

#endif