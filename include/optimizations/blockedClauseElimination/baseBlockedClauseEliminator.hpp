#ifndef BASE_BLOCKED_CLAUSE_ELIMINATIOR_HPP
#define BASE_BLOCKED_CLAUSE_ELIMINATIOR_HPP

#include <dimacs/problemDefinition.hpp>
#include <optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp>

#include <optional>
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

		// Adepted from: https://devblogs.microsoft.com/oldnewthing/20190619-00/?p=102599
		template <typename Container, typename ElementType = std::decay_t<decltype(*begin(std::declval<Container>()))>>
		[[nodiscard]] bool doesEveryClauseInGenericResolutionEnvironmentContainerFullfillLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral, const Container& containerOfIndicesOfClausesContainingNegatedBlockingLiteral) const
		{
			return std::distance(containerOfIndicesOfClausesContainingNegatedBlockingLiteral.cbegin(), containerOfIndicesOfClausesContainingNegatedBlockingLiteral.cend())
				&& std::none_of(
					containerOfIndicesOfClausesContainingNegatedBlockingLiteral.cbegin(),
					containerOfIndicesOfClausesContainingNegatedBlockingLiteral.cend(),
					[&](const ElementType clauseIndex)
					{
						const dimacs::ProblemDefinition::Clause* referencedClause = problemDefinition->getClauseByIndexInFormula(clauseIndex);
						return !referencedClause || !checkLiteralBlockedCondition(clauseToCheck, potentiallyBlockingLiteral, referencedClause->literals);
					});
		}

		[[nodiscard]] virtual bool doesEveryClauseInResolutionEnvironmentFullfillLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral) const = 0;
		[[nodiscard]] static bool checkLiteralBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, long potentiallyBlockingLiteral, const std::vector<long>& literalOfClauseInResolutionEnvironment);
	};
}

#endif