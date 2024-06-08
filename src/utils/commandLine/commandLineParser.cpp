#include "utils/commandLine/commandLineParser.hpp"

using namespace commandLineUtils;

bool CommandLineParser::digest(int argc, char* argv[])
{
	if (argc < 0)
		return false;

	if (!argc)
		return true;

	const std::size_t numArguments = argc;
	for (std::size_t i = 0; i < numArguments; ++i)
	{
		if (i + 1 < numArguments)
			commandLineOptions.emplace(std::make_pair(argv[i], argv[i + 1]));
	}
	return true;
}

bool CommandLineParser::tryGetStringValue(const std::string& optionKey, std::optional<std::string>& containerForFetchedValue) const
{
	if (!commandLineOptions.count(optionKey))
		return false;

	containerForFetchedValue = commandLineOptions.at(optionKey);
	return true;
}

bool CommandLineParser::tryGetUnsignedNumericValue(const std::string& optionKey, std::optional<std::size_t>& containerForFetchedValue, std::optional<std::string>* optionalContainerForRawValue) const
{
	if (!commandLineOptions.count(optionKey))
		return false;

	if (const std::optional<std::string> optionalStringifiedNumericValue = commandLineOptions.at(optionKey); optionalStringifiedNumericValue.has_value())
	{
		if (optionalContainerForRawValue)
			*optionalContainerForRawValue = optionalStringifiedNumericValue;

		errno = 0;
		char* p_end{};
		const long long parsedValue = std::strtoll(optionalStringifiedNumericValue->c_str(), &p_end, 10);
		if (!errno && parsedValue && parsedValue <= SIZE_MAX)
			containerForFetchedValue = static_cast<std::size_t>(parsedValue);
	}
	return true;
}
