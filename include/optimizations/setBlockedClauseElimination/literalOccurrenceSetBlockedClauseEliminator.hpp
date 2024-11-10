#ifndef CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <set>

#include "baseSetBlockedClauseEliminator.hpp"
#include "literalOccurrenceBlockingSetCandidateGenerator.hpp"
#include <dimacs/literalOccurrenceLookup.hpp>

namespace setBlockedClauseElimination {
	class LiteralOccurrenceSetBlockedClauseEliminator : public BaseSetBlockedClauseEliminator {
	public:
		using BaseSetBlockedClauseEliminator::determineBlockingSet;

		LiteralOccurrenceSetBlockedClauseEliminator() = delete;
		explicit LiteralOccurrenceSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseSetBlockedClauseEliminator(std::move(problemDefinition))
		{
			candidateGenerator = std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>();
			if (!candidateGenerator)
				throw std::exception("Failed to initialize candidate generator");
		}

		[[nodiscard]] std::optional<std::vector<long>> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction) const override;
		
	protected:
		LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateGenerator;

		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const override;
		[[nodiscard]] static std::unordered_set<long> determineDifferenceSetBetweenClauseAndBlockingSet(const std::vector<long>& clauseLiterals, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& blockingSet);
	};
}
#endif