#ifndef LITERAL_OCCURRENCE_LOOKUP_HPP
#define LITERAL_OCCURRENCE_LOOKUP_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "literalInContainerIndexLookup.hpp"

namespace dimacs {
	class LiteralOccurrenceLookup {
	public:
		using LiteralOccurrenceLookupEntry = std::unordered_set<std::size_t>;

		LiteralOccurrenceLookup() = default;
		explicit LiteralOccurrenceLookup(std::size_t numVariablesToBeRecorded)
			: nVariables(numVariablesToBeRecorded)
		{
			if (!numVariablesToBeRecorded)
				return;

			const std::optional<std::size_t> requiredContainerSizeForLiteralOccurrences = LiteralInContainerIndexLookup::getRequiredTotalSizeOfContainerToStoreRange(nVariables);
			if (!requiredContainerSizeForLiteralOccurrences)
				throw std::invalid_argument("Lookup data structure can handle at most " + std::to_string( LiteralInContainerIndexLookup::getMaximumStorableNumberOfVariables()) + " variables");

			literalOccurrences = std::vector(*requiredContainerSizeForLiteralOccurrences, LiteralOccurrenceLookupEntry());
		}

		[[nodiscard]] std::optional<const LiteralOccurrenceLookupEntry*> operator[](long literal) const;
		[[nodiscard]] std::optional<std::size_t> getNumberOfOccurrencesOfLiteral(long literal) const;
		[[maybe_unused]] bool recordClauseLiteralOccurrences(std::size_t clauseId, const std::vector<long>& clauseLiterals);
		void removeLiteralFromClause(std::size_t clauseId, long literal);

	protected:
		std::size_t nVariables;
		std::vector<LiteralOccurrenceLookupEntry> literalOccurrences;
	};
}

#endif