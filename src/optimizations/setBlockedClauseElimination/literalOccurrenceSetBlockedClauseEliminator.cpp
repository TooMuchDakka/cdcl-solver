#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>

#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"

using namespace setBlockedClauseElimination;

std::optional<std::vector<long>> LiteralOccurrenceSetBlockedClauseEliminator::determineBlockingSet(std::size_t clauseIdxInFormula, BaseBlockingSetCandidateGenerator& candidateGenerator) const
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

		const std::vector<dimacs::ProblemDefinition::Clause*> resolutionEnvironment = determineResolutionEnvironment(*candidateBlockingSet);
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
		return std::vector(candidateBlockingSet->cbegin(), candidateBlockingSet->cend());

	return std::nullopt;
}

// NON-PUBLIC FUNCTIONALITY
std::vector<dimacs::ProblemDefinition::Clause*> LiteralOccurrenceSetBlockedClauseEliminator::determineResolutionEnvironment(const BaseBlockingSetCandidateGenerator::BlockingSetCandidate& potentialBlockingSet) const
{
	std::unordered_set<std::size_t> indicesOfClausesInResolutionEnvironment;
	for (const long literal : potentialBlockingSet)
	{
		if (const std::optional<std::vector<std::size_t>> clausesContainingLiteral = literalOccurrenceLookup[literal]; clausesContainingLiteral.has_value() && !clausesContainingLiteral->empty())
			indicesOfClausesInResolutionEnvironment.insert(clausesContainingLiteral->cbegin(), clausesContainingLiteral->cend());
	}

	std::vector<dimacs::ProblemDefinition::Clause*> resolutionEnvironment;
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
