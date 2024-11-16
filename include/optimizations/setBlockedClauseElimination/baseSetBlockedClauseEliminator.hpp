#ifndef BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP
#define BASE_SET_BLOCKED_CLAUSE_ELIMINATOR_HPP

#include <set>

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
		[[nodiscard]] virtual std::optional<FoundBlockingSet> determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction)
		{
			const dimacs::ProblemDefinition::Clause* dataOfAccessedClause = problemDefinition->getClauseByIndexInFormula(clauseIdxInFormula);
			if (!dataOfAccessedClause || dataOfAccessedClause->literals.size() < 2)
				return std::nullopt;

			const std::vector<long>& clauseLiterals = dataOfAccessedClause->literals;
			candidateGenerator.init(clauseLiterals, problemDefinition->getLiteralOccurrenceLookup(), optionalCandidateSizeRestriction);

			std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> candidateBlockingSet = candidateGenerator.generateNextCandidate();
			bool foundBlockingSet = false;
			while (!foundBlockingSet && candidateBlockingSet.has_value())
			{
				const std::unordered_set<long>& clauseLiteralsAndBlockingSetDifferenceSet = determineDifferenceSetBetweenClauseAndBlockingSet(clauseLiterals, *candidateBlockingSet);
				foundBlockingSet |= doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(clauseLiteralsAndBlockingSetDifferenceSet, *candidateBlockingSet);
				if (!foundBlockingSet)
					candidateBlockingSet = candidateGenerator.generateNextCandidate();
			}
			if (foundBlockingSet && candidateBlockingSet.has_value())
				return BaseSetBlockedClauseEliminator::FoundBlockingSet(candidateBlockingSet->cbegin(), candidateBlockingSet->cend());

			return std::nullopt;
		};

		// TODO: Enable search for blocking set of specific size?

	protected:
		dimacs::ProblemDefinition::ptr problemDefinition;

		[[nodiscard]] virtual std::unordered_set<std::size_t> determineIndicesOfOverlappingClausesForLiteral(long literal) const = 0;
		[[nodiscard]] bool doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(const std::unordered_set<long>& literalsOfDiffSetOfClauseToCheckAndBlockingSet, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const
		{
			// Resolution environment R for a clause C and a given blocking set L is defined as \forall C' \in R: C' \in F \wedge C' \union \neg{L} != 0
			std::unordered_set<std::size_t> alreadyCheckedClauseIndicesInResolutionEnvironment;
			bool didResolutionEnvironmentContaingAtleastOneEntry = false;
			for (const long literal : potentialBlockingSet)
			{
				const std::unordered_set<std::size_t>& indicesOfClauesContainingNegatedLiteral = determineIndicesOfOverlappingClausesForLiteral(-literal);
				didResolutionEnvironmentContaingAtleastOneEntry |= !indicesOfClauesContainingNegatedLiteral.empty();

				for (const std::size_t clauseIdx : indicesOfClauesContainingNegatedLiteral)
				{
					if (alreadyCheckedClauseIndicesInResolutionEnvironment.count(clauseIdx))
						continue;

					const dimacs::ProblemDefinition::Clause* dataOfClause = problemDefinition->getClauseByIndexInFormula(clauseIdx);
					if (!dataOfClause)
						throw std::out_of_range("Could not determine data for clause with idx " + std::to_string(clauseIdx) + " in formula");

					if (!isClauseSetBlocked(literalsOfDiffSetOfClauseToCheckAndBlockingSet, *dataOfClause, potentialBlockingSet))
						return false;

					alreadyCheckedClauseIndicesInResolutionEnvironment.emplace(clauseIdx);
				}
			}
			return didResolutionEnvironmentContaingAtleastOneEntry;
		}
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

		[[nodiscard]] static std::unordered_set<long> determineDifferenceSetBetweenClauseAndBlockingSet(const std::vector<long>& clauseLiterals, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& blockingSet)
		{
			std::unordered_set<long> differenceSet;
			std::set<long, std::less<>> orderedBlockingSet(blockingSet.cbegin(), blockingSet.cend());
			std::set_difference(
				clauseLiterals.cbegin(), clauseLiterals.cend(),
				orderedBlockingSet.cbegin(), orderedBlockingSet.cend(),
				std::inserter(differenceSet, differenceSet.begin()));
			return differenceSet;
		}
	};
}
#endif