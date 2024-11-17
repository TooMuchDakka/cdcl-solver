#ifndef BASE_BLOCKED_CLAUSE_ELIMINATIOR_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATIOR_HPP

#include <dimacs/problemDefinition.hpp>
#include <optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp>

#include <optional>
#include <unordered_set>

namespace blockedClauseElimination
{
	class BaseBlockedClauseEliminator {
	public:
		virtual ~BaseBlockedClauseEliminator() = default;
		BaseBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
			: problemDefinition(std::move(problemDefinition)) {}

		[[nodiscard]] std::optional<long> determineBlockingLiteralOfClause(std::size_t clauseIndexInFormula, BlockingLiteralGenerator& blockingLiteralGenerator) const;

	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;

		[[nodiscard]] virtual std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const = 0;
		[[nodiscard]] bool doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(const std::vector<long>& clauseLiterals, long potentiallyBlockingLiteral) const;
		[[nodiscard]] static bool checkLiteralBlockedCondition(const std::unordered_set<long >& literalLookupToClauseToCheck, long potentiallyBlockingLiteral, const std::vector<long>& literalOfClauseInResolutionEnvironment);
		[[nodiscard]] static std::unordered_set<long> constructDifferenceSetBetweenClauseAndPotentiallyBlockingLiteral(const std::vector<long>& clauseLiterals, long potentiallyBlockingLiteral);
	};
}

#endif