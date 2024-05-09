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

		[[nodiscard]] std::optional<ProblemDefinition::ptr> readProblemFromFile(const std::string& dimacsFilePath, std::vector<std::string>* optionalFoundErrors);

		DimacsParser(): recordFoundErrors(false), foundErrorsDuringCurrentParsingAttempt(false) {}
	protected:
		bool recordFoundErrors;
		bool foundErrorsDuringCurrentParsingAttempt;
		std::vector<std::string> foundErrors;

		void recordError(std::size_t line, std::size_t column, const std::string& errorText);
		void resetInternals(bool shouldFoundErrorsBeRecorded);
		[[nodiscard]] static std::size_t skipCommentLines(std::ifstream& inputFileStream);
		[[nodiscard]] static std::vector<std::string> splitStringAtDelimiter(const std::string& stringToSplit, char delimiter);
		[[nodiscard]] static std::optional<long> tryConvertStringToLong(const std::string& stringToConvert, std::string* optionalFoundError);
		[[nodiscard]] static std::optional<std::pair<std::size_t, std::size_t>> processProblemDefinitionLine(std::ifstream& inputFileStream, std::string* optionalFoundError);
		[[nodiscard]] static std::optional<dimacs::ProblemDefinition::Clause> parseClauseDefinition(std::ifstream& inputFileStream, std::size_t numDeclaredLiterals, std::size_t numDeclaredClauses, std::string* optionalFoundErrors);
	};
}
#endif