#ifndef BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

namespace setBlockedClauseElimination {
	class BaseSetBlockedClauseEliminator {
	public:
		using FoundBlockingSet = std::vector<long>;
		virtual ~BaseSetBlockedClauseEliminator() = default;

		// TODO: Pure literal elimination in parser or problemDefinition class?
		explicit BaseSetBlockedClauseEliminator(dimacs::ProblemDefinition::ptr problemDefinition)
		{
			if (!problemDefinition)
				throw std::invalid_argument("Problem definition cannot be null");

			this->problemDefinition = std::move(problemDefinition);
		}

		// TODO: Blocking set can also only contain one literal, currently we are assuming that the set always holds two elements
		[[nodiscard]] virtual std::optional<FoundBlockingSet> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator) const = 0;

		// TODO: Enable search for blocking set of specific size?

	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;
		
		[[nodiscard]] virtual std::vector<dimacs::ProblemDefinition::Clause*> determineResolutionEnvironment(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const = 0;
		[[nodiscard]] static bool isClauseSetBlocked(const std::unordered_set<long>& literalsOfDiffSetOfClauseToCheckAndBlockingSet, const dimacs::ProblemDefinition::Clause& clauseInResolutionEnvironment, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) {
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