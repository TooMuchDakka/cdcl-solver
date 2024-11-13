#ifndef CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define CLAUSE_LITERAL_LOOKUP_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "baseSetBlockedClauseEliminator.hpp"
#include "literalOccurrenceBlockingSetCandidateGenerator.hpp"
#include <dimacs/literalOccurrenceLookup.hpp>

namespace setBlockedClauseElimination {
	class LiteralOccurrenceSetBlockedClauseEliminator : public BaseSetBlockedClauseEliminator {
	public:
		using BaseSetBlockedClauseEliminator::determineBlockingSet;

		LiteralOccurrenceSetBlockedClauseEliminator() = delete;
		explicit LiteralOccurrenceSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseSetBlockedClauseEliminator(std::move(problemDefinition)) {}

	protected:
		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const override
		{
			if (const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry* indicesOfClauesContainingNegatedLiteral = problemDefinition->getLiteralOccurrenceLookup()[literal].value_or(nullptr); indicesOfClauesContainingNegatedLiteral)
				return *indicesOfClauesContainingNegatedLiteral;
			return {};
		}
	};
}
#endif