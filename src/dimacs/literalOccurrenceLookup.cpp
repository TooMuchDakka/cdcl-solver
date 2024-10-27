#include "dimacs/literalOccurrenceLookup.hpp"

#include <stdexcept>
#include <string>

using namespace dimacs;

std::optional<std::vector<std::size_t>> LiteralOccurrenceLookup::operator[](long literal) const
{
	if (const std::size_t lookupIndexForLiteral = getLiteralIndexInLookup(literal); lookupIndexForLiteral)
		return std::make_optional<std::vector<std::size_t>>({ literalOccurrences.at(lookupIndexForLiteral).begin(), literalOccurrences.at(lookupIndexForLiteral).end() });

	return std::nullopt;
}

std::optional<std::size_t> LiteralOccurrenceLookup::getNumberOfOccurrencesOfLiteral(long literal) const
{
	if (const std::size_t lookupIndexForLiteral = getLiteralIndexInLookup(literal); lookupIndexForLiteral)
		return literalOccurrences.at(lookupIndexForLiteral).size();

	return std::nullopt;
}


void LiteralOccurrenceLookup::removeLiteralFromClause(std::size_t clauseId, long literal) noexcept
{
	if (const std::size_t lookupIndexForLiteral = getLiteralIndexInLookup(literal); lookupIndexForLiteral)
		literalOccurrences.at(lookupIndexForLiteral).erase(clauseId);
}

// NON-PUBLIC FUNCTIONALITY
void LiteralOccurrenceLookup::initialize(const ProblemDefinition& problemDefinition)
{
	if (nVariables >= (SIZE_MAX / 2))
		throw std::invalid_argument("Lookup data structure can handle at most " + std::to_string(SIZE_MAX - 1) + " variables");

	if (!problemDefinition.getNumVariablesInFormula())
		return;

	nVariables = problemDefinition.getNumVariablesInFormula();
	literalOccurrences = std::vector((nVariables * 2) + 1, std::unordered_set<std::size_t>());

	const std::size_t numClauses = problemDefinition.getNumClauses();
	for (std::size_t i = 0; i < numClauses; ++i)
	{
		const std::optional<const ProblemDefinition::Clause*> lastAccessedClause = problemDefinition.getClauseByIndexInFormula(i);
		if (!lastAccessedClause.has_value())
			throw std::out_of_range("Could not fetched clause with index " + std::to_string(i));

		const std::vector<long>& clauseLiterals = lastAccessedClause.value()->literals;
		for (const long literal : clauseLiterals)
		{
			const auto lookupIndexForLiteral = getLiteralIndexInLookup(literal);
			if (!lookupIndexForLiteral)
				throw std::out_of_range("Could not determine index in lookup for literal " + std::to_string(literal));

			literalOccurrences[lookupIndexForLiteral].emplace(i);
		}
	}
}

std::size_t LiteralOccurrenceLookup::getLiteralIndexInLookup(long literal) const noexcept
{
	if (!literal || std::abs(literal) > static_cast<long>(nVariables))
		return 0;

	if (literal < 0)
		return -literal;

	return nVariables + static_cast<std::size_t>(literal);
}