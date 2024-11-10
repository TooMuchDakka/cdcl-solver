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

		[[nodiscard]] virtual std::optional<FoundBlockingSet> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator)
		{
			return determineBlockingSet(clauseIdxInFormula, candidateGenerator, std::nullopt);
		}
		[[nodiscard]] virtual std::optional<FoundBlockingSet> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction) const = 0;

		// TODO: Enable search for blocking set of specific size?

	protected:
		//struct DiffSetOfClauseAndBlockingSet
		//{
		//	std::unordered_set<long> remainingLiteralsOfClause;
		//	long minimumLiteralValueOfClause;
		//	long maximumLiteralValueOfClause;
		//};

		//dimacs::ProblemDefinition::ptr problemDefinition;

		//[[nodiscard]] virtual std::vector<const dimacs::ProblemDefinition::Clause*> determineResolutionEnvironment(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const = 0;
		//[[nodiscard]] static bool isClauseSetBlocked(const DiffSetOfClauseAndBlockingSet& diffSetOfClauseAndBlockingSet, const dimacs::ProblemDefinition::Clause& clauseInResolutionEnvironment, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) {
		//	// A clause C is blocked by a set L in a formula F iff. forall C' \in F: C' \union L != 0: C\L \union NOT(L) \union C' is a tautology

		// TODO: Check whether a condition can be defined that is usable to determine whether the given clause of the resolution environment could contain a literal of the difference set instead of having to potentially iterate over all literals of said clause
		//	if (clauseInResolutionEnvironment.literals.size() > 1)
		//	{
		//		const long smallestLiteralOfClauseInResolutionEnvironment = clauseInResolutionEnvironment.literals.front();
		//		const long largestLiteralOfClauseInResolutionEnvironemnt = clauseInResolutionEnvironment.literals.back();


		//		if ((clauseInResolutionEnvironment.literals.front() < 0 && diffSetOfClauseAndBlockingSet.maximumLiteralValueOfClause < clauseInResolutionEnvironment.literals.front())
		//			|| clauseInResolutionEnvironment.
		//	}

		//	if (diffSetOfClauseAndBlockingSet.maximumLiteralValueOfClause > clauseInResolutionEnvironment.literals.)

		//		return std::any_of(
		//			clauseInResolutionEnvironment.literals.cbegin(),
		//			clauseInResolutionEnvironment.literals.cend(),
		//			[&potentialBlockingSet, &literalsOfDiffSetOfClauseToCheckAndBlockingSet](const long literal)
		//			{
		//				if (!potentialBlockingSet.count(-literal))
		//					return literalsOfDiffSetOfClauseToCheckAndBlockingSet.count(-literal) > 0;
		//				return false;
		//			}
		//		);
		//}

		dimacs::ProblemDefinition::ptr problemDefinition;

		[[nodiscard]] virtual bool doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(const std::unordered_set<long>& literalsOfDiffSetOfClauseToCheckAndBlockingSet, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const = 0;
		[[nodiscard]] static bool isClauseSetBlocked(const std::unordered_set<long>& literalsOfDiffSetOfClauseToCheckAndBlockingSet, const dimacs::ProblemDefinition::Clause& clauseInResolutionEnvironment, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) {
			// A clause C is blocked by a set L in a formula F iff. forall C' \in F: C' \union L != 0: C\L \union NOT(L) \union C' is a tautology
			return std::any_of(
				clauseInResolutionEnvironment.literals.cbegin(),
				clauseInResolutionEnvironment.literals.cend(),
				[&potentialBlockingSet, &literalsOfDiffSetOfClauseToCheckAndBlockingSet](const long literal)
				{
					if (!potentialBlockingSet.count(-literal))
						return literalsOfDiffSetOfClauseToCheckAndBlockingSet.count(-literal) > 0;
					return false;
				}
			);
		}

		[[nodiscard]] static bool canCheckForBlockingSetBePerformed(const dimacs::ProblemDefinition::Clause& clause) noexcept {
			return clause.literals.size() > 1;
		}
	};
}
#endif