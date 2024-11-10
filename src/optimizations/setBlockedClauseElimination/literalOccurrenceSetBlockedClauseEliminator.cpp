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
std::unordered_set<long> LiteralOccurrenceSetBlockedClauseEliminator::determineDifferenceSetBetweenClauseAndBlockingSet(const std::vector<long>& clauseLiterals, const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& blockingSet)
{
	std::unordered_set<long> differenceSet;
	std::set<long, std::less<>> orderedBlockingSet(blockingSet.cbegin(), blockingSet.cend());
	std::set_difference(
		clauseLiterals.cbegin(), clauseLiterals.cend(),
		orderedBlockingSet.cbegin(), orderedBlockingSet.cend(),
		std::inserter(differenceSet, differenceSet.begin()));
	return differenceSet;
}

std::unordered_set<std::size_t> LiteralOccurrenceSetBlockedClauseEliminator::determineIndicesOfOverlappingClausesForLiteral(long literal) const
{
	if (const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry* indicesOfClauesContainingNegatedLiteral = problemDefinition->getLiteralOccurrenceLookup()[-literal].value_or(nullptr); indicesOfClauesContainingNegatedLiteral)
		return *indicesOfClauesContainingNegatedLiteral;
	return {};
}