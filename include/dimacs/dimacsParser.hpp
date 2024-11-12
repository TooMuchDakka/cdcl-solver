#ifndef DIMACS_PARSER_HPP
#define DIMACS_PARSER_HPP

#include <optional>
#include <string>
#include <dimacs/problemDefinition.hpp>
#include <vector>

namespace dimacs
{
	class DimacsParser
	{
	public:
		using ptr = std::unique_ptr<DimacsParser>;
		struct ParserConfiguration
		{
			bool performUnitPropagation;
			bool recordParsingErrors;
		};

		struct ProcessingError
		{
			struct Position
			{
				std::size_t line;
				std::size_t column;
			};
			std::optional<Position> position;
			std::string text;

			ProcessingError() = default;

			ProcessingError(std::string text)
				: position(std::nullopt), text(std::move(text)) {}

			ProcessingError(std::size_t line, std::size_t column, std::string text)
				: position(Position({line, column})), text(std::move(text)) {}
		};

		struct ParseResult
		{
			std::optional<ProblemDefinition::ptr> formula;
			std::vector<ProcessingError> errors;
			bool determinedAnyErrors;
			bool wasFormulaDeterminedToBeUnsat;

			ParseResult()
				: determinedAnyErrors(true), wasFormulaDeterminedToBeUnsat(false) {}
		};

		[[nodiscard]] ParseResult readProblemFromFile(const std::string& dimacsFilePath);
		[[nodiscard]] ParseResult readProblemFromString(const std::string& dimacsContent);

		DimacsParser(ParserConfiguration configuration)
			: foundErrorsDuringCurrentParsingAttempt(false), configuration(configuration) {}

		DimacsParser()
			: DimacsParser(ParserConfiguration({true, true})) {}
	protected:
		struct ProblemDefinitionConfiguration
		{
			std::size_t numVariables;
			std::size_t numClauses;
		};

		bool foundErrorsDuringCurrentParsingAttempt;
		std::vector<ProcessingError> foundErrors;
		ParserConfiguration configuration;

		void recordError(std::size_t line, std::size_t column, const std::string& errorText);
		void resetInternals();

		[[nodiscard]] std::optional<ProblemDefinition::ptr> parseDimacsContent(std::basic_istream<char>& stream, bool& wasFormulaDeterminedToBeUnsat);
		[[maybe_unused]] static bool removeClausesSatisfiedByUnitPropagation(ProblemDefinition& problemDefinition, long literal);
		[[nodiscard]] static std::size_t skipCommentLines(std::basic_istream<char>& inputStream);
		[[nodiscard]] static std::vector<std::string_view> splitStringAtDelimiter(const std::string_view& stringToSplit, char delimiter);
		[[nodiscard]] static std::optional<long> tryConvertStringToLong(const std::string_view& stringToConvert, ProcessingError* optionalFoundError);
		[[nodiscard]] static std::optional<ProblemDefinitionConfiguration> processProblemDefinitionLine(std::basic_istream<char>& inputStream, ProcessingError* optionalFoundError);
		[[nodiscard]] static std::optional<ProblemDefinition::Clause> parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDefinedVariablesInCnf, const ProblemDefinition& variableValueLookupGateway, ProcessingError* optionalFoundErrors, bool& wasClauseDeterminedToBeUnsat);
	};

	inline std::ostream& operator<<(std::ostream& os, const dimacs::DimacsParser::ProcessingError& processingError)
	{
		if (!processingError.position.has_value())
			os << "--line: UNKNOWN col: UNKNOWN | " + processingError.text;
		else
			os << "--line: " + std::to_string(processingError.position->line) + " col: " + std::to_string(processingError.position->column) + " | " + processingError.text;
		// write obj to stream
		return os;
	}
}
#endif