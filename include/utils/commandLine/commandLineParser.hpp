#ifndef COMMAND_LINE_PARSER_HPP
#define COMMAND_LINE_PARSER_HPP
#include <map>
#include <optional>
#include <string>

namespace commandLineUtils {
	class CommandLineParser {
	public:
		CommandLineParser() = default;
		[[maybe_unused]] bool digest(int argc, char* argv[]);
		[[nodiscard]] bool tryGetStringValue(const std::string& optionKey, std::optional<std::string>& containerForFetchedValue) const;
		[[nodiscard]] bool tryGetUnsignedNumericValue(const std::string& optionKey, std::optional<std::size_t>& containerForFetchedValue, std::optional<std::string>* optionalContainerForRawValue) const;

	protected:
		std::map<std::string, std::optional<std::string>> commandLineOptions;
	};
}
#endif