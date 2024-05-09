#include "dimacs/dimacsParser.hpp"

#include <fstream>
#include <string>


std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::readProblemFromFile(const std::string& dimacsFilePath, std::vector<std::string>* optionalFoundErrors)
{
	resetInternals(optionalFoundErrors);
	std::ifstream inputFileStream(dimacsFilePath);
	if (!inputFileStream.is_open())
	{
		recordError(0, 0, "Could not open file " + dimacsFilePath);
		return std::nullopt;
	}

	std::size_t currProcessedLine = skipCommentLines(inputFileStream);

	std::string foundErrorDuringProcessingOfProblemDefinitionLine;
	const std::optional<std::pair<std::size_t, std::size_t>> problemDefinitionData = processProblemDefinitionLine(inputFileStream, &foundErrorDuringProcessingOfProblemDefinitionLine);
	if (!problemDefinitionData)
	{
		recordError(currProcessedLine, 0, foundErrorDuringProcessingOfProblemDefinitionLine);
		return std::nullopt;
	}
	++currProcessedLine;

	const std::size_t numLiterals = problemDefinitionData->first;
	const std::size_t numClauses = problemDefinitionData->second;

	auto problemDefinition = std::make_unique<ProblemDefinition>(numLiterals, numClauses);
	if (!problemDefinition)
	{
		recordError(0, 0, "Failed to initialize problem definition");
		return std::nullopt;
	}
	std::size_t currNumProcessedClauses = 0;
	std::string clauseProcessingError;

	for (const std::optional<ProblemDefinition::Clause>& parsedClause = parseClauseDefinition(inputFileStream, numLiterals, numClauses, &clauseProcessingError); parsedClause.has_value();) {
		if (currNumProcessedClauses > numClauses)
		{
			recordError(currProcessedLine, 0, "More than " + std::to_string(numClauses) + " clauses defined");
			return std::nullopt;
		}

		if (!clauseProcessingError.empty())
		{
			recordError(currProcessedLine, 0, clauseProcessingError);
			return std::nullopt;
		}
		++currNumProcessedClauses;
		problemDefinition->addClause(currNumProcessedClauses++, *parsedClause);
	}

	if (!clauseProcessingError.empty())
	{
		recordError(currProcessedLine, 0, clauseProcessingError);
		return std::nullopt;
	}

	if (!foundErrorsDuringCurrentParsingAttempt)
		return std::move(problemDefinition);
	return std::nullopt;
}

void dimacs::DimacsParser::resetInternals(bool shouldFoundErrorsBeRecorded)
{
	recordFoundErrors = shouldFoundErrorsBeRecorded;
	foundErrorsDuringCurrentParsingAttempt = false;
	foundErrors.clear();
}


void dimacs::DimacsParser::recordError(std::size_t line, std::size_t column, const std::string& errorText)
{
	foundErrorsDuringCurrentParsingAttempt = true;
	if (recordFoundErrors)
		foundErrors.emplace_back("--line: " + std::to_string(line) + " col: " + std::to_string(column) + " | " + errorText);
}


inline std::vector<std::string> dimacs::DimacsParser::splitStringAtDelimiter(const std::string& stringToSplit, char delimiter)
{
	std::size_t lastFoundDelimiterPosition = 0;
	std::vector<std::string> splitStringParts;
	for (const std::size_t foundDelimiterPosition = stringToSplit.find(delimiter, lastFoundDelimiterPosition); foundDelimiterPosition != std::string::npos;)
	{
		const std::size_t splitPartLength = (foundDelimiterPosition + 1) - lastFoundDelimiterPosition;
		lastFoundDelimiterPosition = foundDelimiterPosition + 1;
		splitStringParts.emplace_back(stringToSplit.substr(lastFoundDelimiterPosition, splitPartLength));
	}
	return splitStringParts;
}

inline std::size_t dimacs::DimacsParser::skipCommentLines(std::ifstream& inputFileStream)
{
	std::size_t numCommentLines = 0;

	bool continueProcessing = true;
	std::string tmpBufferForLine;
	while (continueProcessing)
	{
		++numCommentLines;
		std::getline(inputFileStream, tmpBufferForLine);
		continueProcessing = inputFileStream.good() && tmpBufferForLine._Starts_with("c");
		if (continueProcessing)
			++numCommentLines;
	}
	return numCommentLines;
}

inline std::optional<long> dimacs::DimacsParser::tryConvertStringToLong(const std::string& stringToConvert, std::string* optionalFoundError)
{
	if (stringToConvert == "0")
		return 0;

	char* endPointer = nullptr;
	const long convertedNumericValue = std::strtol(stringToConvert.data(), &endPointer, 10);

	if (errno == ERANGE || !convertedNumericValue)
	{
		if (optionalFoundError)
			*optionalFoundError = "Conversion of string " + stringToConvert + " to numeric value failed";
		return std::nullopt;
	}
	return convertedNumericValue;
}

inline std::optional<std::pair<std::size_t, std::size_t>> dimacs::DimacsParser::processProblemDefinitionLine(std::ifstream& inputFileStream, std::string* optionalFoundError)
{
	std::string readProblemLineDefinition;
	if (!std::getline(inputFileStream, readProblemLineDefinition) || !readProblemLineDefinition._Starts_with("p"))
	{
		if (optionalFoundError)
			*optionalFoundError = "Expected line in format: p cnf <NUM_LITERALS> <NUM_CLAUSES>";
		return std::nullopt;
	}

	if (const std::vector<std::string> splitProblemDefinitionLineData = splitStringAtDelimiter(readProblemLineDefinition, ' '); splitProblemDefinitionLineData.size() == 4
		|| splitProblemDefinitionLineData.at(1) != "cnf")
	{
		const std::optional<long> numUserDefinedLiterals = tryConvertStringToLong(splitProblemDefinitionLineData.at(2), nullptr);
		const std::optional<long> numUserDefinedClauses = tryConvertStringToLong(splitProblemDefinitionLineData.at(3), nullptr);
		if (!numUserDefinedLiterals || !numUserDefinedClauses)
			return std::nullopt;

		return std::make_pair(*numUserDefinedLiterals, *numUserDefinedClauses);
	}

	if (optionalFoundError)
		*optionalFoundError = "Expected line in format: p cnf <NUM_LITERALS> <NUM_CLAUSES>";
	return std::nullopt;
}

inline std::optional<dimacs::ProblemDefinition::Clause> dimacs::DimacsParser::parseClauseDefinition(std::ifstream& inputFileStream, std::size_t numDeclaredLiterals, std::size_t numDeclaredClauses, std::string* optionalFoundErrors)
{

	std::string clauseDefinition;
	if (!std::getline(inputFileStream, clauseDefinition))
		return std::nullopt;

	dimacs::ProblemDefinition::Clause clause;
	for (const std::string& clauseLiteralData : splitStringAtDelimiter(clauseDefinition, ' '))
	{
		std::string stringToNumberConversionError;
		const std::optional<long> parsedClauseLiteral = tryConvertStringToLong(clauseLiteralData, &stringToNumberConversionError);
		if (!parsedClauseLiteral)
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = stringToNumberConversionError;
			return std::nullopt;
		}

		if (!*parsedClauseLiteral || (*parsedClauseLiteral < 0 && std::abs(*parsedClauseLiteral) >= static_cast<long>(numDeclaredLiterals)) || (*parsedClauseLiteral && *parsedClauseLiteral >= static_cast<long>(numDeclaredLiterals)))
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = "Literal " + std::to_string(*parsedClauseLiteral) + " was out of range, valid range is [-" + std::to_string(numDeclaredLiterals) + ", -1] v [1, " + std::to_string(numDeclaredLiterals) + "]";
			return std::nullopt;
		}
		clause.literals.emplace_back(*parsedClauseLiteral);
	}
	return clause;
}
