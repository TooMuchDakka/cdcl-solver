#include "benchmark/commandLineArgumentParser.hpp"

#include <stdexcept>

using namespace utils;

// Exceptionless conversion to long int: https://stackoverflow.com/questions/26080829/detecting-strtol-failure/26083517#26083517
std::optional<int> CommandLineArgumentParser::CommandLineArgumentRegistration::tryGetArgumentValueAsInteger() const
{
	if (!optionalArgumentValue.has_value() || !argumentType.has_value() || *argumentType != CommandLineArgumentType::Integer)
		return std::nullopt;
	return std::stoi(*optionalArgumentValue);
}

void CommandLineArgumentParser::registerCommandLineArgument(const std::string& commandLineKey, const CommandLineArgumentRegistration& registrationData)
{
	if (registeredCommandLineArguments.count(commandLineKey))
		throw std::invalid_argument("Duplicate registration of command line argument using key " + commandLineKey);

	registeredCommandLineArguments.emplace(commandLineKey, registrationData);
}

void CommandLineArgumentParser::processCommandLineArguments(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		const std::string stringifiedCommandLineArgument = argv[i];
		if (!registeredCommandLineArguments.count(stringifiedCommandLineArgument))
			continue;

		CommandLineArgumentRegistration& existingRegistration = registeredCommandLineArguments.at(stringifiedCommandLineArgument);
		if (existingRegistration.wasFoundInCommandLineArgument)
			throw std::invalid_argument("Duplicate definition of command line argument " + stringifiedCommandLineArgument);

		existingRegistration.wasFoundInCommandLineArgument = true;
		if (!existingRegistration.argumentType.has_value())
			continue;

		if (i + 1 == argc)
			throw std::invalid_argument("Required value for argument " + stringifiedCommandLineArgument + " was missing");

		const std::string stringifiedCommandLineArgumentValue = argv[++i];
		if (stringifiedCommandLineArgumentValue.empty())
			throw std::invalid_argument("Invalid value '" + stringifiedCommandLineArgumentValue + "' for argument " + stringifiedCommandLineArgument);

		if (existingRegistration.argumentType == CommandLineArgumentType::Integer)
			const int _ = std::stoi(stringifiedCommandLineArgumentValue);

		existingRegistration.optionalArgumentValue = stringifiedCommandLineArgumentValue;
	}

	for (const auto& [key, registration] : registeredCommandLineArguments)
	{
		if (registration.isOptional)
			continue;

		if (!registration.wasFoundInCommandLineArgument)
			throw std::logic_error("Required argument " + key + " was not found");
	}
}

std::optional<CommandLineArgumentParser::CommandLineArgumentRegistration> CommandLineArgumentParser::getValueOfArgument(const std::string& commandLineKey) const
{
	if (!registeredCommandLineArguments.count(commandLineKey))
		return std::nullopt;

	return registeredCommandLineArguments.at(commandLineKey);
}

const std::unordered_map<std::string, CommandLineArgumentParser::CommandLineArgumentRegistration>& CommandLineArgumentParser::getRegisteredCommandLineArguments() const noexcept
{
	return registeredCommandLineArguments;
}
