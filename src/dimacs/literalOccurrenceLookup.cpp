#include "dimacs/literalOccurrenceLookup.hpp"

#include <algorithm>
#include <stdexcept>

using namespace dimacs;

std::optional<std::vector<std::size_t>> LiteralOccurrenceLookup::operator[](long literal) const
{
	if (const std::optional<std::size_t> lookupIndexForLiteral = LiteralInContainerIndexLookup::getIndexInContainer(literal, nVariables); lookupIndexForLiteral.value_or(0))
		return std::make_optional<std::vector<std::size_t>>({ literalOccurrences.at(*lookupIndexForLiteral).begin(), literalOccurrences.at(*lookupIndexForLiteral).end() });

	return std::nullopt;
}

std::optional<std::size_t> LiteralOccurrenceLookup::getNumberOfOccurrencesOfLiteral(long literal) const
{
	if (const std::optional<std::size_t> lookupIndexForLiteral = LiteralInContainerIndexLookup::getIndexInContainer(literal, nVariables); lookupIndexForLiteral.value_or(0))
		return literalOccurrences.at(*lookupIndexForLiteral).size();

	return std::nullopt;
}

void LiteralOccurrenceLookup::removeLiteralFromClause(std::size_t clauseId, long literal)
{
	if (const std::optional<std::size_t> lookupIndexForLiteral = LiteralInContainerIndexLookup::getIndexInContainer(literal, nVariables); lookupIndexForLiteral.value_or(0))
		literalOccurrences.at(*lookupIndexForLiteral).erase(clauseId);
}

bool LiteralOccurrenceLookup::recordClauseLiteralOccurrences(std::size_t clauseId, const std::vector<long>& clauseLiterals)
{
	return std::all_of(
		clauseLiterals.cbegin(),
		clauseLiterals.cend(),
		[&](const long literal)
		{
			if (const std::optional<std::size_t> lookupIndexForLiteral = LiteralInContainerIndexLookup::getIndexInContainer(literal, nVariables); lookupIndexForLiteral.value_or(0))
			{
				literalOccurrences.at(*lookupIndexForLiteral).emplace(clauseId);
				return true;
			}
			return false;
		}
	);
}