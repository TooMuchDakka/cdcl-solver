#include "dimacs/dimacsParser.hpp"

#include <fstream>
#include <sstream>
#include <string>

using namespace dimacs;
DimacsParser::ParseResult DimacsParser::readProblemFromFile(const std::string& dimacsFilePath)
{
	ParseResult parseResult;
	/*
	 * The data return by tellg(..) as well as seekg is only correct, if the content from the file is read in binary mode.
	 * see: https://stackoverflow.com/questions/47508335/what-is-tellg-in-file-handling-in-c-and-how-does-it-work
	 */
	std::ifstream inputFileStream(dimacsFilePath, std::ifstream::binary);
	if (!inputFileStream.is_open())
		recordError(0, 0, "Could not open file " + dimacsFilePath);
	else
		parseResult.formula = parseDimacsContent(inputFileStream, parseResult.wasFormulaDeterminedToBeUnsat);

	parseResult.determinedAnyErrors = foundErrorsDuringCurrentParsingAttempt;
	parseResult.errors = foundErrors;
	return parseResult;
}

DimacsParser::ParseResult DimacsParser::readProblemFromString(const std::string& dimacsContent)
{
	std::istringstream buffer(dimacsContent);

	ParseResult parseResult;
	parseResult.formula = parseDimacsContent(buffer, parseResult.wasFormulaDeterminedToBeUnsat);
	parseResult.determinedAnyErrors = foundErrorsDuringCurrentParsingAttempt;
	parseResult.errors = foundErrors;
	return parseResult;
}

std::optional<ProblemDefinition::ptr> DimacsParser::parseDimacsContent(std::basic_istream<char>& stream, bool& wasFormulaDeterminedToBeUnsat)
{
	resetInternals();
	std::size_t currProcessedLine = skipCommentLines(stream) + 1;

	ProcessingError foundErrorDuringProcessingOfProblemDefinitionLine;
	ProcessingError* temporaryProcessingErrorContainer = configuration.recordParsingErrors ? &foundErrorDuringProcessingOfProblemDefinitionLine : nullptr;
	const std::optional<ProblemDefinitionConfiguration> problemDefinitionConfiguration = processProblemDefinitionLine(stream, temporaryProcessingErrorContainer);
	if (!problemDefinitionConfiguration)
	{
		recordError(currProcessedLine, 0, foundErrorDuringProcessingOfProblemDefinitionLine.text);
		return std::nullopt;
	}

	auto problemDefinition = std::make_unique<ProblemDefinition>(problemDefinitionConfiguration->numVariables, problemDefinitionConfiguration->numClauses);
	if (!problemDefinition)
	{
		recordError(0, 0, "Failed to initialize problem definition");
		return std::nullopt;
	}

	std::size_t processedClauseCounter = 0;
	ProcessingError clauseParsingError;
	temporaryProcessingErrorContainer = configuration.recordParsingErrors ? &clauseParsingError : nullptr;

	bool continueProcessing;
	do
	{
		++currProcessedLine;
		std::optional<ProblemDefinition::Clause> parsedClause = parseClauseDefinition(stream, problemDefinitionConfiguration->numVariables, *problemDefinition, temporaryProcessingErrorContainer, wasFormulaDeterminedToBeUnsat);
		if (!clauseParsingError.text.empty())
			recordError(currProcessedLine, 0, clauseParsingError.text);

		++processedClauseCounter;
		continueProcessing = stream.peek() && !stream.eof();

		if (processedClauseCounter > problemDefinitionConfiguration->numClauses)
			break;

		if (!parsedClause.has_value())
			continue;

		if (configuration.performUnitPropagation && parsedClause->literals.size() == 1)
		{
			const long unitPropagatedLiteral = parsedClause->literals.front();
			const std::size_t numAssignmentsPrioToUnitPropagation = problemDefinition->getPastAssignments().size();
			const ProblemDefinition::PropagationResult propagationResult = problemDefinition->propagate(unitPropagatedLiteral);

			if (propagationResult == ProblemDefinition::Conflict)
			{
				wasFormulaDeterminedToBeUnsat = true;
			}
			else if (propagationResult == ProblemDefinition::Ok)
			{
				const std::vector<ProblemDefinition::PastAssignment>& pastAssignments = problemDefinition->getPastAssignments();
				if (pastAssignments.size() <= numAssignmentsPrioToUnitPropagation)
					return std::nullopt;

				const std::size_t numPerformedVariableAssignments = pastAssignments.size() - numAssignmentsPrioToUnitPropagation;
				for (std::size_t i = 0; i < numPerformedVariableAssignments; ++i)
				{
					const long l = pastAssignments.at(numAssignmentsPrioToUnitPropagation + i).assignedLiteral;
					if (!removeClausesSatisfiedByUnitPropagation(*problemDefinition, l))
						recordError(currProcessedLine, 0, "Error during removal of clauses containing unit propagated literal " + std::to_string(l));
					if (!problemDefinition->removeLiteralFromClausesOfFormula(-l))
						recordError(currProcessedLine, 0, "Error during removal of literal " + std::to_string(-l) + " from clauses of formula");
				}
			}
			else
			{
				recordError(currProcessedLine - 1, 0, "Error during unit propagation of literal " + std::to_string(unitPropagatedLiteral));
			}
		}
		else if (parsedClause->isTautology())
			recordError(currProcessedLine, 0, "Formula is expected to contain no tautologies");
		else
			problemDefinition->addClause(processedClauseCounter - 1, *parsedClause);
	} while (continueProcessing);

	if (processedClauseCounter != problemDefinitionConfiguration->numClauses)
		recordError(currProcessedLine, 0, "Expected formula to contain " + std::to_string(problemDefinitionConfiguration->numClauses) + " clauses but " + std::to_string(processedClauseCounter) + " were parsed");

	// TODO: Local variable elimination
	if (!foundErrorsDuringCurrentParsingAttempt)
		return std::move(problemDefinition);

	return std::nullopt;
}

bool DimacsParser::removeClausesSatisfiedByUnitPropagation(ProblemDefinition& problemDefinition, long literal)
{
	const LiteralOccurrenceLookup& literalOccurrenceLookup = problemDefinition.getLiteralOccurrenceLookup();
	const std::optional<const dimacs::LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry*> lookupEntry = literalOccurrenceLookup[literal];
	if (!lookupEntry.has_value())
		return false;

	const LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry clausesContainingLiteral = *lookupEntry ? **lookupEntry : LiteralOccurrenceLookup::LiteralOccurrenceLookupEntry();
	for (const std::size_t clauseIndex : clausesContainingLiteral)
	{
		if (!problemDefinition.removeClause(clauseIndex))
			return false;
	}
	return true;
}

void DimacsParser::resetInternals()
{
	foundErrorsDuringCurrentParsingAttempt = false;
	foundErrors.clear();
}


void DimacsParser::recordError(std::size_t line, std::size_t column, const std::string& errorText)
{
	foundErrorsDuringCurrentParsingAttempt = true;
	if (configuration.recordParsingErrors)
		foundErrors.emplace_back(line, column, errorText);
}

inline std::vector<std::string_view> DimacsParser::splitStringAtDelimiter(const std::string_view& stringToSplit, char delimiter)
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

inline std::optional<long> DimacsParser::tryConvertStringToLong(const std::string_view& stringToConvert, ProcessingError* optionalFoundError)
{
	if (stringToConvert == "0")
		return 0;

	char* endPointer = nullptr;
	const long convertedNumericValue = std::strtol(stringToConvert.data(), &endPointer, 10);

	if (errno == ERANGE || !convertedNumericValue)
	{
		if (optionalFoundError)
			*optionalFoundError = ProcessingError("Conversion of string " + std::string(stringToConvert) + " to numeric value failed");
		return std::nullopt;
	}
	return convertedNumericValue;
}

inline std::optional<DimacsParser::ProblemDefinitionConfiguration> DimacsParser::processProblemDefinitionLine(std::basic_istream<char>& inputStream, ProcessingError* optionalFoundError)
{
	std::string readProblemLineDefinition;
	bool parsingFailed = !std::getline(inputStream, readProblemLineDefinition);
	parsingFailed |= !readProblemLineDefinition.empty() && readProblemLineDefinition.front() != 'p';

	const auto& splitProblemDefinitionConfigurationLineData = !parsingFailed ? splitStringAtDelimiter(readProblemLineDefinition, ' ') : std::vector<std::string_view>();
	if (splitProblemDefinitionConfigurationLineData.empty() || splitProblemDefinitionConfigurationLineData.at(1) != "cnf" || splitProblemDefinitionConfigurationLineData.size() != 4)
	{
		if (optionalFoundError)
			*optionalFoundError = ProcessingError("Expected line in format: p cnf <NUM_LITERALS> <NUM_CLAUSES> but was actually " + readProblemLineDefinition);
		return std::nullopt;
	}

	const std::optional<long> userDefinedNumberOfVariables = tryConvertStringToLong(splitProblemDefinitionConfigurationLineData.at(2), optionalFoundError);
	const std::optional<long> userDefinedNumberOfClauses = tryConvertStringToLong(splitProblemDefinitionConfigurationLineData.at(3), optionalFoundError);
	if (!userDefinedNumberOfVariables || !userDefinedNumberOfClauses)
		return std::nullopt;

	if (*userDefinedNumberOfVariables < 0)
	{
		if (optionalFoundError)
			*optionalFoundError = ProcessingError("Processed integer value " + std::to_string(*userDefinedNumberOfVariables) + " must be larger than 0");
		return std::nullopt;
	}
	if (*userDefinedNumberOfClauses < 0)
	{
		if (optionalFoundError)
			*optionalFoundError = ProcessingError("Processed integer value " + std::to_string(*userDefinedNumberOfClauses) + " must be larger than 0");
		return std::nullopt;
	}
	return ProblemDefinitionConfiguration({ static_cast<std::size_t>(*userDefinedNumberOfVariables), static_cast<std::size_t>(*userDefinedNumberOfClauses) });
}

std::optional<ProblemDefinition::Clause> DimacsParser::parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDefinedVariablesInCnf, const ProblemDefinition& variableValueLookupGateway, ProcessingError* optionalFoundErrors, bool& wasClauseDeterminedToBeUnsat)
{
	std::string clauseDefinition;
	if (!std::getline(inputStream, clauseDefinition))
		return std::nullopt;

	const std::vector<std::string_view>& stringifiedClauseLiterals = splitStringAtDelimiter(clauseDefinition, ' ');
	if (stringifiedClauseLiterals.empty())
		return std::nullopt;

	ProblemDefinition::Clause clause;
	clause.literals.reserve(stringifiedClauseLiterals.size() - 1);
	clause.satisified = false;

	bool wasRequiredEndDelimiterDefined = false;
	bool doesCurrentVariableAssignmentSatisfyClause = false;
	bool couldDetermineCurrentClauseIsUnsat = false;

	for (auto stringifiedClauseLiteralIterator = stringifiedClauseLiterals.begin(); stringifiedClauseLiteralIterator < stringifiedClauseLiterals.end(); ++stringifiedClauseLiteralIterator)
	{
		const std::optional<long> clauseLiteral = tryConvertStringToLong(*stringifiedClauseLiteralIterator, optionalFoundErrors);
		if (!clauseLiteral.has_value())
			return std::nullopt;

		if (std::abs(*clauseLiteral) > static_cast<long>(numDefinedVariablesInCnf))
		{
			if (optionalFoundErrors)
				*optionalFoundErrors = ProcessingError("Literal " + std::to_string(*clauseLiteral) + " was out of range, valid range is [-" + std::to_string(numDefinedVariablesInCnf) + ", -1] v [1, " + std::to_string(numDefinedVariablesInCnf) + "]");
			return std::nullopt;
		}

		wasRequiredEndDelimiterDefined |= clauseLiteral.value_or(1) == 0;
		if (wasRequiredEndDelimiterDefined)
		{
			if (stringifiedClauseLiteralIterator != std::prev(stringifiedClauseLiterals.end()))
			{
				if (optionalFoundErrors)
					*optionalFoundErrors = ProcessingError("Clause must define literal 0 as its closing delimiter");
				return std::nullopt;
			}
		}
		else
		{
			const ProblemDefinition::VariableValue currentValueOfVariable = variableValueLookupGateway.getValueOfVariable(std::abs(*clauseLiteral)).value_or(ProblemDefinition::VariableValue::Unknown);
			doesCurrentVariableAssignmentSatisfyClause |= currentValueOfVariable == ProblemDefinition::determineSatisfyingAssignmentForLiteral(*clauseLiteral);

			if (!doesCurrentVariableAssignmentSatisfyClause)
			{
				couldDetermineCurrentClauseIsUnsat = currentValueOfVariable == ProblemDefinition::determineConflictingAssignmentForLiteral(*clauseLiteral);
				if (currentValueOfVariable == ProblemDefinition::Unknown)
					clause.literals.emplace_back(*clauseLiteral);
			}
			else
				couldDetermineCurrentClauseIsUnsat = false;
		}
	}

	if (!wasRequiredEndDelimiterDefined)
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = ProcessingError("Clause must define literal 0 as its closing delimiter");
		return std::nullopt;
	}

	wasClauseDeterminedToBeUnsat |= couldDetermineCurrentClauseIsUnsat;
	if (!doesCurrentVariableAssignmentSatisfyClause)
	{
		clause.sortLiteralsAscendingly();
		return clause;
	}
	return std::nullopt;
}