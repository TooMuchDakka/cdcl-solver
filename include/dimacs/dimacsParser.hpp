#ifndef DIMACS_PARSER_HPP
#define DIMACS_PARSER_HPP

#include <optional>
#include <string>
#include <vector>

#include "localClauseLiteralRemover.hpp"
#include "problemDefinition.hpp"

namespace dimacs
{
	class DimacsParser
	{
	public:
		using ptr = std::unique_ptr<DimacsParser>;

		struct PreprocessingOptimizationsConfig
		{
			bool localClauseLiteralRemovalEnabled;
			bool singleLiteralClauseRemovalEnabled;
		};

		[[nodiscard]] std::optional<ProblemDefinition::ptr> readProblemFromFile(const std::string& dimacsFilePath, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig);
		[[nodiscard]] std::optional<ProblemDefinition::ptr> readProblemFromString(const std::string& dimacsContent, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig);

		DimacsParser(): recordFoundErrors(false), foundErrorsDuringCurrentParsingAttempt(false) {}
	protected:
		bool recordFoundErrors;
		bool foundErrorsDuringCurrentParsingAttempt;
		std::vector<std::string> foundErrors;

		void recordError(std::size_t line, std::size_t column, const std::string& errorText);
		void resetInternals(bool shouldFoundErrorsBeRecorded);

		[[nodiscard]] std::optional<ProblemDefinition::ptr> parseDimacsContent(std::basic_istream<char>& stream, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig);
		[[nodiscard]] static std::size_t skipCommentLines(std::basic_istream<char>& inputStream);
		[[nodiscard]] static std::vector<std::string_view> splitStringAtDelimiter(const std::string_view& stringToSplit, char delimiter);
		[[nodiscard]] static std::optional<long> tryConvertStringToLong(const std::string_view& stringToConvert, std::string* optionalFoundError);
		[[nodiscard]] static std::optional<std::pair<std::size_t, std::size_t>> processProblemDefinitionLine(std::basic_istream<char>& inputStream, std::string* optionalFoundError);
		[[nodiscard]] static std::optional<ProblemDefinition::Clause> parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDeclaredLiterals, std::string* optionalFoundErrors);
		static void removeLocalClauseLiteralsFromFormula(ProblemDefinition& problemDefinition, const localClauseLiteralRemoval::LocalClauseLiteralRemover& localClauseLiteralRemover);
		[[nodiscard]] static bool doesClauseOnlyDefineOneLiteralWithSamePolarity(const ProblemDefinition::Clause& clause);
	};
}
#endif