#ifndef COMMAND_LINE_ARGUMENT_PARSER_HPP
#define COMMAND_LINE_ARGUMENT_PARSER_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <ostream>

namespace utils {
	class CommandLineArgumentParser {
	public:
		enum CommandLineArgumentType
		{
			String,
			Integer
		};

		struct CommandLineArgumentRegistration
		{
			bool isOptional;
			bool wasFoundInCommandLineArgument;
			std::optional<std::string> optionalArgumentValue;
			std::optional<CommandLineArgumentType> argumentType;

			[[nodiscard]] std::optional<int> tryGetArgumentValueAsInteger() const;

			[[nodiscard]] static CommandLineArgumentRegistration createIntegerArgument()
			{
				return CommandLineArgumentRegistration({ false, false, std::nullopt, CommandLineArgumentType::Integer });
			}
			[[nodiscard]] static CommandLineArgumentRegistration createStringArgument()
			{
				return CommandLineArgumentRegistration({ false, false, std::nullopt, CommandLineArgumentType::String });
			}
			[[nodiscard]] static CommandLineArgumentRegistration createValueLessArgument()
			{
				return CommandLineArgumentRegistration({ false, false, std::nullopt, std::nullopt });
			}

			[[nodiscard]] CommandLineArgumentRegistration& asOptionalArgument()
			{
				isOptional = true;
				return *this;
			}
		};

		void registerCommandLineArgument(const std::string& commandLineKey, const CommandLineArgumentRegistration& registrationData);
		void processCommandLineArguments(int argc, char* argv[]);

		[[nodiscard]] std::optional<CommandLineArgumentRegistration> getValueOfArgument(const std::string& commandLineKey) const;
		[[nodiscard]] const std::unordered_map<std::string, CommandLineArgumentRegistration>& getRegisteredCommandLineArguments() const noexcept;
	protected:
		std::unordered_map<std::string, CommandLineArgumentRegistration> registeredCommandLineArguments;
	};

	inline std::ostream& operator<<(std::ostream& os, const CommandLineArgumentParser& commandLineArgumentParser)
	{
		const std::unordered_map<std::string, CommandLineArgumentParser::CommandLineArgumentRegistration>& registeredArguments = commandLineArgumentParser.getRegisteredCommandLineArguments();
		for (const auto& [key, registration] : registeredArguments)
		{
			std::string stringifiedArgumentType = "None";
			if (registration.argumentType.has_value())
			{
				switch (registration.argumentType.value())
				{
					case CommandLineArgumentParser::Integer:
						stringifiedArgumentType = "Integer";
						break;
					case CommandLineArgumentParser::String:
						stringifiedArgumentType = "String";
						break;
				}
			}
			os << "KEY: " + key + " | OPTIONAL: " + std::to_string(registration.isOptional) + " | TYPE: " + stringifiedArgumentType + " | VALUE: " + registration.optionalArgumentValue.value_or("") + "\n" ;
		}
		return os;
	}
}

#endif