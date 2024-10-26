#ifndef BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class BaseSetBlockedClauseEliminator {
	public:
		virtual ~BaseSetBlockedClauseEliminator() = default;

		// TODO: Pure literal elimination in parser or problemDefinition class?
		BaseSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition, std::unique_ptr<BaseSetBlockedClauseEliminator> blockingSetCandidateGenerator)
			: problemDefinition(std::move(problemDefinition)), blockingSetCandidateGenerator(std::move(blockingSetCandidateGenerator)) {}

		[[nodiscard]] virtual std::optional<std::vector<long>> determineBlockingSet(std::size_t clauseIdxInFormula) const = 0;
		virtual void ignoreSetBlockedClause(std::size_t clauseIdxInFormula) = 0;

		// TODO: Enable search for blocking set of specific size?

	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
		std::unique_ptr<BaseSetBlockedClauseEliminator> blockingSetCandidateGenerator;
		const dimacs::ProblemDefinition::Clause EMPTY_CLAUSE;

		[[nodiscard]] virtual bool isClauseSetBlocked(std::size_t clauseIdxInFormula, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const = 0;
		[[nodiscard]] static bool isClauseSetBlocked(const std::unordered_set<long> literalsOfDiffSetOfClauseToCheckAndBlockingSet, const dimacs::ProblemDefinition::Clause& clauseInResolutionEnvironment, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) {
			// A clause C is blocked by a set L in a formula F iff. forall C' \in F: C' \union L != 0: C\L \union NOT(L) \union C' is a tautology 
			return std::any_of(
				clauseInResolutionEnvironment.literals.cbegin(),
				clauseInResolutionEnvironment.literals.cend(),
				[&potentialBlockingSet, &literalsOfDiffSetOfClauseToCheckAndBlockingSet](const long literalOfClauseInResolutionEnvironment) {
					return potentialBlockingSet.count(-literalOfClauseInResolutionEnvironment)
						|| literalsOfDiffSetOfClauseToCheckAndBlockingSet.count(-literalOfClauseInResolutionEnvironment); }
			);
		}

		[[nodiscard]] static inline bool canCheckForBlockingSetBePerformed(const dimacs::ProblemDefinition::Clause& clause) noexcept {
			return clause.literals.size() > 1;
		}
	};
}
#endif