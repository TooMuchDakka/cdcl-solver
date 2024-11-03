#ifndef CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <stdexcept>

#include "baseSetBlockedClauseEliminator.hpp"
#include <dimacs/literalOccurrenceLookup.hpp>

#include "literalOccurrenceBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class LiteralOccurrenceSetBlockedClauseEliminator : public BaseSetBlockedClauseEliminator {
	public:
		using BaseSetBlockedClauseEliminator::determineBlockingSet;

		LiteralOccurrenceSetBlockedClauseEliminator() = delete;
		explicit LiteralOccurrenceSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseSetBlockedClauseEliminator(std::move(problemDefinition)), literalOccurrenceLookup(*this->problemDefinition)
		{
			candidateGenerator = std::make_unique<LiteralOccurrenceBlockingSetCandidateGenerator>();
			if (!candidateGenerator)
				throw std::exception("Failed to initialize candidate generator");
		}

		[[nodiscard]] std::optional<std::vector<long>> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction) const override;
		
	protected:
		LiteralOccurrenceBlockingSetCandidateGenerator::ptr candidateGenerator;
		dimacs::LiteralOccurrenceLookup literalOccurrenceLookup;

		[[nodiscard]] std::vector<const dimacs::ProblemDefinition::Clause*> determineResolutionEnvironment(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const override;
	};
}
#endif