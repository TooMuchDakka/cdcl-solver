#include "dimacs/literalOccurrenceLookup.hpp"

#include <algorithm>
#include <stdexcept>

using namespace dimacs;

std::optional<const LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry*> LiteralOccurrenceLookup::operator[](long literal) const
{
	if (const std::optional<std::size_t> lookupIndexForLiteral = LiteralInContainerIndexLookup::getIndexInContainer(literal, nVariables); lookupIndexForLiteral.value_or(0))
	{
		if (literalOccurrences.at(*lookupIndexForLiteral).empty())
			return nullptr;
		return std::make_optional(&literalOccurrences.at(*lookupIndexForLiteral));
	}
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