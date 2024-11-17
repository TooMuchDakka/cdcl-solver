#ifndef LITERAL_OCCURRENCE_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define LITERAL_OCCURRENCE_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "optimizations/blockedClauseElimination/baseBlockedClauseEliminator.hpp"

namespace blockedClauseElimination {
	class LiteralOccurrenceBlockedClauseEliminator : public BaseBlockedClauseEliminator {
	public:
		LiteralOccurrenceBlockedClauseEliminator() = delete;
		explicit LiteralOccurrenceBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: BaseBlockedClauseEliminator(std::move(problemDefinition)) {}

	protected:
		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const override;
	};
}

#endif