#ifndef LITERAL_IN_CONTAINER_INDEX_LOOKUP_HPP
#define LITERAL_IN_CONTAINER_INDEX_LOOKUP_HPP

#include <cstddef>
#include <optional>

namespace dimacs {
	struct LiteralInContainerIndexLookup {
		[[nodiscard]] static std::optional<std::size_t> getRequiredTotalSizeOfContainerToStoreRange(std::size_t numVariablesToStore) {
			if (numVariablesToStore >= (SIZE_MAX / 2))
				return std::nullopt;

			return (numVariablesToStore * 2) + 1;
		}

		[[nodiscard]] static std::optional<std::size_t> getIndexInContainer(long literal, std::size_t nVariables) {
			if (!literal)
				return std::nullopt;

			if (std::abs(literal) > static_cast<long>(nVariables))
				return 0;

			if (literal < 0)
				return -literal;

			return nVariables + static_cast<std::size_t>(literal);
		}

		[[nodiscard]] static constexpr std::size_t getMaximumStorableNumberOfVariables()
		{
			return SIZE_MAX - 1;
		}
	};
}

#endif