#include <dimacsParser.hpp>
#include <localClauseLiteralRemover.hpp>
#include <fstream>
#include <sstream>
#include <string>

using namespace dimacs;

std::optional<ProblemDefinition::ptr> DimacsParser::readProblemFromFile(const std::string& dimacsFilePath, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig)
{
	resetInternals(optionalFoundErrors);
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
	return parseDimacsContent(inputFileStream, optionalFoundErrors, preprocessingOptimizationsConfig);
}

std::optional<ProblemDefinition::ptr> DimacsParser::readProblemFromString(const std::string& dimacsContent, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig)
{
	std::istringstream buffer(dimacsContent);
	resetInternals(optionalFoundErrors);
	return parseDimacsContent(buffer, optionalFoundErrors, preprocessingOptimizationsConfig);
}

std::optional<ProblemDefinition::ptr> DimacsParser::parseDimacsContent(std::basic_istream<char>& stream, std::vector<std::string>* optionalFoundErrors, const PreprocessingOptimizationsConfig& preprocessingOptimizationsConfig)
{
	std::optional<std::unique_ptr<localClauseLiteralRemoval::LocalClauseLiteralRemover>> optionalLocalClauseLiteralRemover;
	
	std::size_t currProcessedLine = 1 + skipCommentLines(stream);

	std::string foundErrorDuringProcessingOfProblemDefinitionLine;
	const std::optional<std::pair<std::size_t, std::size_t>> problemDefinitionData = processProblemDefinitionLine(stream, &foundErrorDuringProcessingOfProblemDefinitionLine);
	if (!problemDefinitionData)
	{
		recordError(currProcessedLine, 0, foundErrorDuringProcessingOfProblemDefinitionLine);
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}
	++currProcessedLine;

	const std::size_t numLiterals = problemDefinitionData->first;
	const std::size_t numClauses = problemDefinitionData->second;

	auto problemDefinition = std::make_unique<ProblemDefinition>(numLiterals, numClauses);
	if (!problemDefinition)
	{
		recordError(0, 0, "Failed to initialize problem definition");
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}

	if (preprocessingOptimizationsConfig.localClauseLiteralRemovalEnabled)
	{
		optionalLocalClauseLiteralRemover = std::make_unique<localClauseLiteralRemoval::LocalClauseLiteralRemover>(numLiterals);
		if (!*optionalLocalClauseLiteralRemover)
		{
			recordError(0, 0, "Failed to initialize optional local clause literal remover");
			if (optionalFoundErrors)
				*optionalFoundErrors = foundErrors;
			return std::nullopt;
		}
	}

	std::size_t currNumProcessedClauses = 0;
	std::string clauseProcessingError;

	bool continueProcessingClauses = true;
	std::optional<ProblemDefinition::Clause::ptr> parsedClause = parseClauseDefinition(stream, numLiterals, &clauseProcessingError);
	while (parsedClause.has_value() && continueProcessingClauses) {
		if (currNumProcessedClauses == numClauses)
		{
			recordError(currProcessedLine, 0, "More than " + std::to_string(numClauses) + " clauses defined");
			continueProcessingClauses = false;
		}

		if (!clauseProcessingError.empty())
		{
			recordError(currProcessedLine, 0, clauseProcessingError);
			continueProcessingClauses = false;
		}

		if (parsedClause->get()->literals.size() == 1 && preprocessingOptimizationsConfig.singleLiteralClauseRemovalEnabled)
		{
			problemDefinition->fixVariableAssignment(parsedClause->get()->literals.front());
			++currNumProcessedClauses;
		}
		else
		{
			problemDefinition->addClause(currNumProcessedClauses++, *parsedClause);
		}
		clauseProcessingError = "";
		++currProcessedLine;

		if (preprocessingOptimizationsConfig.localClauseLiteralRemovalEnabled && optionalLocalClauseLiteralRemover.has_value())
		{
			for (const long clauseLiteral : parsedClause->get()->literals)
			{
				optionalLocalClauseLiteralRemover->get()->recordLiteralUsageInClause(clauseLiteral, currNumProcessedClauses - 1);
			}
		}
		parsedClause = parseClauseDefinition(stream, numLiterals, &clauseProcessingError);
	}

	if (currNumProcessedClauses < numClauses)
		recordError(currProcessedLine, 0, "Expected " + std::to_string(numClauses) + " but only " + std::to_string(currNumProcessedClauses) + " were defined");

	if (!clauseProcessingError.empty())
		recordError(currProcessedLine, 0, clauseProcessingError);

	if (foundErrorsDuringCurrentParsingAttempt)
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}

	if (optionalLocalClauseLiteralRemover.has_value())
	{
		removeLocalClauseLiteralsFromFormula(*problemDefinition, **optionalLocalClauseLiteralRemover);
	}
	return std::move(problemDefinition);
}


void DimacsParser::resetInternals(bool shouldFoundErrorsBeRecorded)
{
	recordFoundErrors = shouldFoundErrorsBeRecorded;
	foundErrorsDuringCurrentParsingAttempt = false;
	foundErrors.clear();
}


void DimacsParser::recordError(std::size_t line, std::size_t column, const std::string& errorText)
{
	foundErrorsDuringCurrentParsingAttempt = true;
	if (recordFoundErrors)
		foundErrors.emplace_back("--line: " + std::to_string(line) + " col: " + std::to_string(column) + " | " + errorText);
}


inline std::vector<std::string> DimacsParser::splitStringAtDelimiter(const std::string& stringToSplit, char delimiter)
{
	std::size_t lastFoundDelimiterPosition = 0;
	std::vector<std::string> splitStringParts;

	std::size_t foundDelimiterPosition = stringToSplit.find(delimiter, lastFoundDelimiterPosition);
	while(!(foundDelimiterPosition == std::string::npos))
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

inline std::size_t DimacsParser::skipCommentLines(std::basic_istream<char>& inputStream)
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

inline std::optional<long> DimacsParser::tryConvertStringToLong(const std::string& stringToConvert, std::string* optionalFoundError)
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

inline std::optional<std::pair<std::size_t, std::size_t>> DimacsParser::processProblemDefinitionLine(std::basic_istream<char>& inputStream, std::string* optionalFoundError)
{
	std::string readProblemLineDefinition;
	if (!std::getline(inputStream, readProblemLineDefinition) || readProblemLineDefinition.rfind('p', 0))
	{
		if (optionalFoundError)
			*optionalFoundError = "Expected line in format: p cnf <NUM_LITERALS> <NUM_CLAUSES> but was actually " + readProblemLineDefinition;
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

inline std::optional<ProblemDefinition::Clause::ptr> DimacsParser::parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDeclaredLiterals, std::string* optionalFoundErrors)
{
	std::string clauseDefinition;
	if (!std::getline(inputStream, clauseDefinition))
		return std::nullopt;

	ProblemDefinition::Clause::ptr clause = std::make_shared<ProblemDefinition::Clause>();
	if (!clause)
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = "Could not allocate memory for clause";
		return std::nullopt;
	}

	const std::vector<std::string> stringifiedClauseLiterals = splitStringAtDelimiter(clauseDefinition, ' ');
	if (stringifiedClauseLiterals.empty())
		return clause;

	std::size_t clauseLiteralIdx = 0;
	for (const std::string& clauseLiteralData : stringifiedClauseLiterals)
	{
		std::string stringToNumberConversionError;
		const std::optional<long> parsedClauseLiteral = tryConvertStringToLong(clauseLiteralData, &stringToNumberConversionError);
		if (!parsedClauseLiteral)
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = stringToNumberConversionError;
			return std::nullopt;
		}

		if (!*parsedClauseLiteral && clauseLiteralIdx < stringifiedClauseLiterals.size() - 1)
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = "Literal 0 cannot only be used as the closing delimiter of a clause but was actually defines as the " + std::to_string(clauseLiteralIdx) + "-th literal of the clause";
			return std::nullopt;
		}

		if (std::abs(*parsedClauseLiteral) > static_cast<long>(numDeclaredLiterals))
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = "Literal " + std::to_string(*parsedClauseLiteral) + " was out of range, valid range is [-" + std::to_string(numDeclaredLiterals) + ", -1] v [1, " + std::to_string(numDeclaredLiterals) + "]";
			return std::nullopt;
		}
		
		clause->literals.emplace_back(*parsedClauseLiteral);
		++clauseLiteralIdx;
	}

	if (clause->literals.back())
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = "Clause must define literal 0 as its closing delimiter";
		return std::nullopt;
	}

	clause->literals.pop_back();
	clause->sortLiterals();
	return clause;
}

void DimacsParser::removeLocalClauseLiteralsFromFormula(ProblemDefinition& problemDefinition, const localClauseLiteralRemoval::LocalClauseLiteralRemover& localClauseLiteralRemover)
{
	std::vector<long> removedLiterals;
	const std::shared_ptr<std::vector<ProblemDefinition::Clause::ptr>> formulaClauses = problemDefinition.getClauses();
	for (auto formulaClauseIterator = formulaClauses->begin(); formulaClauseIterator !=formulaClauses->end();)
	{
		const std::size_t declaredNumLiteralsOfClause = formulaClauseIterator->get()->literals.size();
		const std::vector<long> removedLiteralsOfClause = localClauseLiteralRemover.removeLocalLiteralsFromClause(**formulaClauseIterator);
		if (removedLiteralsOfClause.size() == declaredNumLiteralsOfClause
			|| (declaredNumLiteralsOfClause - removedLiteralsOfClause.size() && doesClauseOnlyDefineOneLiteralWithSamePolarity(**formulaClauseIterator)))
		{
			formulaClauseIterator = formulaClauses->erase(formulaClauseIterator);
		}
		else
		{
			++formulaClauseIterator;
		}

		if (!removedLiteralsOfClause.empty())
		{
			for (const long removedLiteral : removedLiteralsOfClause)
			{
				problemDefinition.fixVariableAssignment(removedLiteral);
			}
		}
	}
}

bool DimacsParser::doesClauseOnlyDefineOneLiteralWithSamePolarity(const ProblemDefinition::Clause& clause)
{
	if (clause.literals.empty())
		return false;

	const long lastFoundClauseLiteral = clause.literals.front();
	for (std::size_t i = 1; i  < clause.literals.size(); ++i)
	{
		if (clause.literals.at(i) != lastFoundClauseLiteral)
			return false;
	}
	return true;
}