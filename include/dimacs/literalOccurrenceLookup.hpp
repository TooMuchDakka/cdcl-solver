#ifndef LITERAL_OCCURRENCE_LOOKUP_HPP
#define LITERAL_OCCURRENCE_LOOKUP_HPP

#include "problemDefinition.hpp"
#include <unordered_set>

namespace dimacs {
	class LiteralOccurrenceLookup {
	public:
		using ptr = std::shared_ptr<LiteralOccurrenceLookup>;
		LiteralOccurrenceLookup(const ProblemDefinition& problemDefinition) : nVariables(0) { initialize(problemDefinition); }

		[[nodiscard]] std::optional<std::vector<std::size_t>> operator[](long literal) const;
		[[nodiscard]] std::optional<std::size_t> getNumberOfOccurrencesOfLiteral(long literal) const;
		void removeLiteralFromClause(std::size_t clauseId, long literal) noexcept;

	protected:
		std::size_t nVariables;
		std::vector<std::unordered_set<std::size_t>> literalOccurrences;

		void initialize(const ProblemDefinition& problemDefinition);
		[[nodiscard]] std::size_t getLiteralIndexInLookup(long literal) const noexcept;

	};
}

#endif