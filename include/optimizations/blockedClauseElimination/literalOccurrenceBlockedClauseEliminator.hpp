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
		[[nodiscard]] bool doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral) const override;
	};
}

#endif