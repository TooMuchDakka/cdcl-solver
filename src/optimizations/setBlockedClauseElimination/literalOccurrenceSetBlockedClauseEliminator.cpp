#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>
#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

using namespace setBlockedClauseElimination;

std::optional<BaseSetBlockedClauseEliminator::FoundBlockingSet> LiteralOccurrenceSetBlockedClauseEliminator::determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator) const
{
	const std::optional<dimacs::ProblemDefinition::Clause*> dataOfAccessedClause = problemDefinition->getClauseByIndexInFormula(clauseIdxInFormula);
	if (!dataOfAccessedClause.has_value() || dataOfAccessedClause.value()->literals.size() < 2)
		return std::nullopt;

	const std::vector<long>& clauseLiterals = dataOfAccessedClause.value()->literals;

	std::unordered_set<long> clauseLiteralAndBlockingDiffSet;
	candidateGenerator.init(clauseLiterals, literalOccurrenceLookup);

	std::optional<BaseBlockingSetCandidateGenerator::BlockingSetCandidate> candidateBlockingSet = candidateGenerator.generateNextCandidate();
	bool foundBlockingSet = false;
	while (!foundBlockingSet && candidateBlockingSet.has_value())
	{
		std::set_difference(
			clauseLiterals.cbegin(), clauseLiterals.cend(),
			candidateBlockingSet->cbegin(), candidateBlockingSet->cend(), 
			std::inserter(clauseLiteralAndBlockingDiffSet, clauseLiteralAndBlockingDiffSet.begin())
		);

		const std::vector<const dimacs::ProblemDefinition::Clause*> resolutionEnvironment = determineResolutionEnvironment(*candidateBlockingSet);
		foundBlockingSet = !resolutionEnvironment.empty() && std::all_of(
			resolutionEnvironment.cbegin(),
			resolutionEnvironment.cend(),
			[&](const dimacs::ProblemDefinition::Clause* clauseInResolutionEnvironment)
			{
				return isClauseSetBlocked(clauseLiteralAndBlockingDiffSet, *clauseInResolutionEnvironment, *candidateBlockingSet);
			});

		if (!foundBlockingSet)
			candidateBlockingSet = candidateGenerator.generateNextCandidate();
	}

	if (candidateBlockingSet.has_value())
		return BaseSetBlockedClauseEliminator::FoundBlockingSet(candidateBlockingSet->cbegin(), candidateBlockingSet->cend());

	return std::nullopt;
}

// NON-PUBLIC FUNCTIONALITY
std::vector<const dimacs::ProblemDefinition::Clause*> LiteralOccurrenceSetBlockedClauseEliminator::determineResolutionEnvironment(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const
{
	// Resolution environment R for a clause C and a given blocking set L is defined as \forall C' \in R: C' \in F \wedge C' \union \neg{L} != 0
	std::unordered_set<std::size_t> indicesOfClausesInResolutionEnvironment;
	for (const long literal : potentialBlockingSet)
	{
		if (const std::optional<std::vector<std::size_t>> clausesContainingLiteral = literalOccurrenceLookup[-literal]; clausesContainingLiteral.has_value() && !clausesContainingLiteral->empty())
			indicesOfClausesInResolutionEnvironment.insert(clausesContainingLiteral->cbegin(), clausesContainingLiteral->cend());
	}

	std::vector<const dimacs::ProblemDefinition::Clause*> resolutionEnvironment;
	resolutionEnvironment.reserve(indicesOfClausesInResolutionEnvironment.size());
	for (const std::size_t idx : indicesOfClausesInResolutionEnvironment)
	{
		const std::optional<dimacs::ProblemDefinition::Clause*> dataOfClause = problemDefinition->getClauseByIndexInFormula(idx);
		if (!dataOfClause)
			throw std::out_of_range("Could not determine data for clause with idx " + std::to_string(idx) + " in formula");

		resolutionEnvironment.emplace_back(*dataOfClause);
	}
	return resolutionEnvironment;
}
