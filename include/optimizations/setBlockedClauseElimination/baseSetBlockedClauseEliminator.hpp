#ifndef BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include "dimacs/problemDefinition.hpp"
#include "optimizations/setBlockedClauseElimination/baseBlockingSetCandidateGenerator.hpp"

#include <set>

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
		[[nodiscard]] virtual std::optional<FoundBlockingSet> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction)
		{
			const dimacs::ProblemDefinition::Clause* dataOfAccessedClause = problemDefinition->getClauseByIndexInFormula(clauseIdxInFormula);
			if (!dataOfAccessedClause)
				return std::nullopt;

			const std::vector<long>& clauseLiterals = dataOfAccessedClause->literals;
			candidateGenerator.init(clauseLiterals, problemDefinition->getLiteralOccurrenceLookup(), optionalCandidateSizeRestriction);

			std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> candidateBlockingSet;
			bool foundBlockingSet = false;
			do
			{
				candidateBlockingSet = candidateGenerator.generateNextCandidate();
				foundBlockingSet |= candidateBlockingSet.has_value() && doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(*dataOfAccessedClause, *candidateBlockingSet);
			} while (!foundBlockingSet && candidateBlockingSet.has_value());

			if (foundBlockingSet && candidateBlockingSet.has_value())
				return FoundBlockingSet(candidateBlockingSet->cbegin(), candidateBlockingSet->cend());

			return std::nullopt;
		}

	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;

		[[nodiscard]] virtual std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const = 0;
		[[nodiscard]] bool doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(const dimacs::ProblemDefinition::Clause& clauseToCheck, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const
		{
			// Resolution environment R for a clause C and a given blocking set L is defined as \forall C' \in R: C' \in F \wedge C' \union \neg{L} != 0
			std::unordered_set<std::size_t> alreadyCheckedClauseIndicesInResolutionEnvironment;
			bool didResolutionEnvironmentContaingAtleastOneEntry = false;
			bool doesConditionHold = true;

			for (auto literalIterator = potentialBlockingSet.begin(); doesConditionHold && literalIterator != potentialBlockingSet.end(); ++literalIterator)
			{
				const std::unordered_set<std::size_t>& indicesOfClauesContainingNegatedLiteral = determineIndicesOfOverlappingClausesForLiteral(-*literalIterator);
				didResolutionEnvironmentContaingAtleastOneEntry |= !indicesOfClauesContainingNegatedLiteral.empty();

				for (auto clauseIndexIterator = indicesOfClauesContainingNegatedLiteral.begin(); doesConditionHold && clauseIndexIterator != indicesOfClauesContainingNegatedLiteral.end(); ++clauseIndexIterator)
				{
					const std::size_t clauseIdx = *clauseIndexIterator;
					const dimacs::ProblemDefinition::Clause* dataOfClause = problemDefinition->getClauseByIndexInFormula(clauseIdx);
					doesConditionHold &= dataOfClause && (!alreadyCheckedClauseIndicesInResolutionEnvironment.count(clauseIdx) ? isClauseSetBlocked(clauseToCheck, *dataOfClause, potentialBlockingSet) : true);
					alreadyCheckedClauseIndicesInResolutionEnvironment.emplace(clauseIdx);
				}
			}
			return didResolutionEnvironmentContaingAtleastOneEntry & doesConditionHold;
		}

		[[nodiscard]] static bool isClauseSetBlocked(const dimacs::ProblemDefinition::Clause& clauseToCheck, const dimacs::ProblemDefinition::Clause& clauseInResolutionEnvironment, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) {
			// A clause C is blocked by a set L in a formula F iff. forall C' \in F: C' \union L != 0: C\L \union NOT(L) \union C' is a tautology
			return std::any_of(
				clauseInResolutionEnvironment.literals.cbegin(),
				clauseInResolutionEnvironment.literals.cend(),
				[&potentialBlockingSet, &clauseToCheck](const long literal)
				{
					// I.	Blocking set contains negation of currently checked literal of clause in resolution environment = Negated blocking set contains currently checked literal of clause in resolution environment
					// II.	C\L contains the negation of a literal of C'
					return potentialBlockingSet.count(literal)
						|| !potentialBlockingSet.count(-literal) && clauseToCheck.containsLiteral(-literal);
				}
			);
		}
	};
}
#endif