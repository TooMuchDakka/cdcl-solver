#include "dimacs/dimacsParser.hpp"

#include <fstream>
#include <sstream>
#include <string>

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::readProblemFromFile(const std::string& dimacsFilePath, std::vector<std::string>* optionalFoundErrors)
{
	/*
	 * The data return by tellg(..) as well as seekg is only correct, if the content from the file is read in binary mode.
	 * see: https://stackoverflow.com/questions/47508335/what-is-tellg-in-file-handling-in-c-and-how-does-it-work
	 */
	std::ifstream inputFileStream(dimacsFilePath, std::ifstream::binary);
	if (!inputFileStream.is_open())
	{
		recordError(0, 0, "Could not open file " + dimacsFilePath);
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}
	return parseDimacsContent(inputFileStream, optionalFoundErrors);
}

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::readProblemFromString(const std::string& dimacsContent, std::vector<std::string>* optionalFoundErrors)
{
	std::istringstream buffer(dimacsContent);
	return parseDimacsContent(buffer, optionalFoundErrors);
}

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::parseDimacsContent(std::basic_istream<char>& stream, std::vector<std::string>* optionalFoundErrors)
{
	resetInternals(optionalFoundErrors);
	std::size_t currProcessedLine = skipCommentLines(stream) + 1;

	std::string foundErrorDuringProcessingOfProblemDefinitionLine = optionalFoundErrors != nullptr ? "" : nullptr;
	const std::optional<ProblemDefinitionConfiguration> problemDefinitionConfiguration = processProblemDefinitionLine(stream, &foundErrorDuringProcessingOfProblemDefinitionLine);
	if (!problemDefinitionConfiguration)
	{
		recordError(currProcessedLine, 0, foundErrorDuringProcessingOfProblemDefinitionLine);
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}
	++currProcessedLine;

	auto problemDefinition = std::make_unique<ProblemDefinition>(problemDefinitionConfiguration->numVariables, problemDefinitionConfiguration->numClauses);
	if (!problemDefinition)
	{
		recordError(0, 0, "Failed to initialize problem definition");
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}

	std::size_t processedClauseCounter = 0;
	std::string clauseParsingError;
	
	
	bool continueProcessing;
	do
	{
		std::optional<ProblemDefinition::Clause> parsedClause = parseClauseDefinition(stream, problemDefinitionConfiguration->numVariables, &clauseParsingError);
		if (!parsedClause.has_value())
			break;

		if (!clauseParsingError.empty() && processedClauseCounter == problemDefinitionConfiguration->numClauses)
		{
			clauseParsingError = "More than " + std::to_string(problemDefinitionConfiguration->numClauses) + " clauses defined";
			recordError(currProcessedLine++, 0, clauseParsingError);
			break;
		}

		if (!clauseParsingError.empty())
		{
			recordError(currProcessedLine++, 0, clauseParsingError);
		}
		else if (!isClauseTautology(*parsedClause))
		{
			if (parsedClause->literals.size() == 1)
			{
				if (problemDefinition->propagate(parsedClause->literals.front()) != ProblemDefinition::Ok)
					recordError(currProcessedLine, 0, "Error during unit propagation of literal " + std::to_string(parsedClause->literals.front()));
			}
			else
			{
				problemDefinition->addClause(processedClauseCounter++, *parsedClause);
			}
		}
		else
			++processedClauseCounter;
		
		continueProcessing = stream.peek() && !stream.eof();
		++currProcessedLine;
	} while (continueProcessing);

	if (processedClauseCounter < problemDefinitionConfiguration->numClauses)
		recordError(currProcessedLine, 0, "Expected " + std::to_string(problemDefinitionConfiguration->numClauses) + " but only " + std::to_string(processedClauseCounter) + " were defined");

	// TODO: Local variable elimination
	if (!foundErrorsDuringCurrentParsingAttempt)
		return std::move(problemDefinition);

	if (optionalFoundErrors)
		*optionalFoundErrors = foundErrors;
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


inline std::vector<std::string_view> dimacs::DimacsParser::splitStringAtDelimiter(const std::string_view& stringToSplit, char delimiter)
{
	if (stringToSplit.empty())
		return {};

	std::size_t lastFoundDelimiterPosition = 0;
	std::vector<std::string_view> splitStringParts;

	std::size_t foundDelimiterPosition = stringToSplit.find(delimiter, lastFoundDelimiterPosition);
	while (foundDelimiterPosition != std::string::npos)
	{
		if (const std::size_t splitPartLength = foundDelimiterPosition - lastFoundDelimiterPosition)
			splitStringParts.emplace_back(stringToSplit.substr(lastFoundDelimiterPosition, splitPartLength));

		lastFoundDelimiterPosition = foundDelimiterPosition + 1;
		foundDelimiterPosition = stringToSplit.find(delimiter, lastFoundDelimiterPosition);
	}

	// Since the last entry has no delimiter successor character, we need to add treat the remaining part of the string as if a 'virtual' delimiter character existed after it only if we have found the delimiter at any prior position in the string
	if (!splitStringParts.empty())
		splitStringParts.emplace_back(stringToSplit.substr(lastFoundDelimiterPosition, stringToSplit.length() - lastFoundDelimiterPosition));

	return splitStringParts;
}

inline std::size_t dimacs::DimacsParser::skipCommentLines(std::basic_istream<char>& inputStream)
{
	std::size_t numCommentLines = 0;

	bool continueProcessing = true;
	std::string tmpBufferForLine;
	while (continueProcessing)
	{
		/*
		 * We need to store the current position in the stream prior to trying to read a new line if the latter turns out to be no comment line.
		 * Otherwise, the line after the last comment line was already consumed and will be "lost"
		 */
		const std::streamoff previousStreamPositionPriorToReadOfNewline = inputStream.tellg();
		std::getline(inputStream, tmpBufferForLine);
		if (!tmpBufferForLine.rfind('c', 0))
		{
			++numCommentLines;
			continueProcessing &= inputStream.good();
		}
		else
		{
			if (inputStream.good())
				inputStream.seekg(previousStreamPositionPriorToReadOfNewline);
			continueProcessing = false;
		}
	}
	return numCommentLines;
}

inline std::optional<long> dimacs::DimacsParser::tryConvertStringToLong(const std::string_view& stringToConvert, std::string* optionalFoundError)
{
	if (stringToConvert == "0")
		return 0;

	char* endPointer = nullptr;
	const long convertedNumericValue = std::strtol(stringToConvert.data(), &endPointer, 10);

	if (errno == ERANGE || !convertedNumericValue)
	{
		if (optionalFoundError)
			*optionalFoundError = "Conversion of string " + std::string(stringToConvert) + " to numeric value failed";
		return std::nullopt;
	}
	return convertedNumericValue;
}

inline std::optional<dimacs::DimacsParser::ProblemDefinitionConfiguration> dimacs::DimacsParser::processProblemDefinitionLine(std::basic_istream<char>& inputStream, std::string* optionalFoundError)
{
	std::string readProblemLineDefinition;
	bool parsingFailed = !std::getline(inputStream, readProblemLineDefinition);
	parsingFailed |= !readProblemLineDefinition.empty() && readProblemLineDefinition.front() == 'p';

	const auto& splitProblemDefinitionConfigurationLineData = !parsingFailed ? splitStringAtDelimiter(readProblemLineDefinition, ' ') : std::vector<std::string_view>();
	if (splitProblemDefinitionConfigurationLineData.empty() || splitProblemDefinitionConfigurationLineData.at(1) != "cnf" || splitProblemDefinitionConfigurationLineData.size() == 4)
	{
		if (optionalFoundError)
			*optionalFoundError = "Expected line in format: p cnf <NUM_LITERALS> <NUM_CLAUSES> but was actually " + readProblemLineDefinition;
		return std::nullopt;
	}

	const std::optional<long> userDefinedNumberOfVariables = tryConvertStringToLong(splitProblemDefinitionConfigurationLineData.at(2), optionalFoundError);
	const std::optional<long> userDefinedNumberOfClauses = tryConvertStringToLong(splitProblemDefinitionConfigurationLineData.at(3), optionalFoundError);
	if (!userDefinedNumberOfVariables || !userDefinedNumberOfClauses)
		return std::nullopt;

	if (*userDefinedNumberOfVariables < 0)
	{
		if (optionalFoundError)
			*optionalFoundError = "Processed integer value " + std::to_string(*userDefinedNumberOfVariables) + " must be larger than 0";
		return std::nullopt;
	}
	if (*userDefinedNumberOfClauses < 0)
	{
		if (optionalFoundError)
			*optionalFoundError = "Processed integer value " + std::to_string(*userDefinedNumberOfClauses) + " must be larger than 0";
		return std::nullopt;
	}


	if (*userDefinedNumberOfVariables > static_cast<long>(SIZE_MAX))
	{
		if (optionalFoundError)
			*optionalFoundError = "Processed integer value " + std::to_string(*userDefinedNumberOfVariables) + " is larger than the maximum storable value of " + std::to_string(SIZE_MAX);
		return std::nullopt;
	}
	if (*userDefinedNumberOfClauses > static_cast<long>(SIZE_MAX))
	{
		if (optionalFoundError)
			*optionalFoundError = "Processed integer value " + std::to_string(*userDefinedNumberOfClauses) + " is larger than the maximum storable value of " + std::to_string(SIZE_MAX);
		return std::nullopt;
	}
	return ProblemDefinitionConfiguration({ static_cast<std::size_t>(*userDefinedNumberOfVariables), static_cast<std::size_t>(*userDefinedNumberOfClauses) });
}

inline std::optional<dimacs::ProblemDefinition::Clause> dimacs::DimacsParser::parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDefinedVariablesInCnf, std::string* optionalFoundErrors)
{
	std::string clauseDefinition;
	if (!std::getline(inputStream, clauseDefinition))
		return std::nullopt;

	const std::vector<std::string_view>& stringifiedClauseLiterals = splitStringAtDelimiter(clauseDefinition, ' ');
	if (stringifiedClauseLiterals.empty())
		return std::nullopt;

	ProblemDefinition::Clause clause;
	clause.literals.reserve(stringifiedClauseLiterals.size() - 1);

	if (clause.literals.back())
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = "Clause must define literal 0 as its closing delimiter";
		return std::nullopt;
	}

	for (auto stringifiedClauseLiteralIterator = stringifiedClauseLiterals.begin(); stringifiedClauseLiteralIterator < stringifiedClauseLiterals.end(); ++stringifiedClauseLiteralIterator)
	{
		const std::optional<long> clauseLiteral = tryConvertStringToLong(*stringifiedClauseLiteralIterator, optionalFoundErrors);
		if (!clauseLiteral.has_value())
			return std::nullopt;

		if (std::abs(*clauseLiteral) > static_cast<long>(numDefinedVariablesInCnf))
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = "Literal " + std::to_string(*clauseLiteral) + " was out of range, valid range is [-" + std::to_string(numDefinedVariablesInCnf) + ", -1] v [1, " + std::to_string(numDefinedVariablesInCnf) + "]";
			return std::nullopt;
		}

		if (!*clauseLiteral && stringifiedClauseLiteralIterator != std::prev(stringifiedClauseLiterals.end(), 2)) 
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = "Clause must define literal 0 as its closing delimiter";
			return std::nullopt;
		}
		clause.literals.emplace_back(*clauseLiteral);
	}
	clause.sortLiterals();
	return clause;
}

bool dimacs::DimacsParser::isClauseTautology(const dimacs::ProblemDefinition::Clause& clause) noexcept
{
	if (clause.literals.size() < 2)
		return false;

	std::size_t forwardIndex = 0;
	std::size_t backwardIndex = clause.literals.size() - 1;
	bool isTautology = false;
	while (forwardIndex < backwardIndex && !isTautology)
		isTautology = clause.literals[forwardIndex++] == -clause.literals[--backwardIndex];

	return isTautology;
}