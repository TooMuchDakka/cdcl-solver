#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>
#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

using namespace setBlockedClauseElimination;

std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> LiteralOccurrenceSetBlockedClauseEliminator::determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator, const std::optional<BaseBlockingSetCandidateGenerator::CandidateSizeRestriction>& optionalCandidateSizeRestriction) const
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
}

// NON-PUBLIC FUNCTIONALITY
bool LiteralOccurrenceSetBlockedClauseEliminator::doesEveryClauseInResolutionEnvironmentFullfillSetBlockedCondition(const std::unordered_set<long>& literalsOfDiffSetOfClauseToCheckAndBlockingSet, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const
{
	// Resolution environment R for a clause C and a given blocking set L is defined as \forall C' \in R: C' \in F \wedge C' \union \neg{L} != 0
	std::unordered_set<std::size_t> alreadyCheckedClauseIndicesInResolutionEnvironment;
	bool didResolutionEnvironmentContaingAtleastOneEntry = false;
	for (const long literal : potentialBlockingSet)
	{
		const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry* indicesOfClauesContainingNegatedLiteral = problemDefinition->getLiteralOccurrenceLookup()[-literal]
			.value_or(nullptr);
		didResolutionEnvironmentContaingAtleastOneEntry |= indicesOfClauesContainingNegatedLiteral != nullptr;
		if (!indicesOfClauesContainingNegatedLiteral)
			continue;

		for (const std::size_t clauseIdx : *indicesOfClauesContainingNegatedLiteral)
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