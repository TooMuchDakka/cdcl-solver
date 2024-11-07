#include "dimacs/dimacsParser.hpp"

#include <fstream>
#include <sstream>
#include <string>

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::readProblemFromFile(const std::string& dimacsFilePath, std::vector<ProcessingError>* optionalFoundErrors)
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

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::readProblemFromString(const std::string& dimacsContent, std::vector<ProcessingError>* optionalFoundErrors)
{
	std::istringstream buffer(dimacsContent);
	return parseDimacsContent(buffer, optionalFoundErrors);
}

std::optional<dimacs::ProblemDefinition::ptr> dimacs::DimacsParser::parseDimacsContent(std::basic_istream<char>& stream, std::vector<ProcessingError>* optionalFoundErrors)
{
	resetInternals(optionalFoundErrors);
	std::size_t currProcessedLine = skipCommentLines(stream) + 1;

	ProcessingError foundErrorDuringProcessingOfProblemDefinitionLine;
	ProcessingError* temporaryProcessingErrorContainer = optionalFoundErrors ? &foundErrorDuringProcessingOfProblemDefinitionLine : nullptr;
	const std::optional<ProblemDefinitionConfiguration> problemDefinitionConfiguration = processProblemDefinitionLine(stream, temporaryProcessingErrorContainer);
	if (!problemDefinitionConfiguration)
	{
		recordError(currProcessedLine, 0, foundErrorDuringProcessingOfProblemDefinitionLine.text);
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}

	auto problemDefinition = std::make_unique<ProblemDefinition>(problemDefinitionConfiguration->numVariables, problemDefinitionConfiguration->numClauses);
	if (!problemDefinition)
	{
		recordError(0, 0, "Failed to initialize problem definition");
		if (optionalFoundErrors)
			*optionalFoundErrors = foundErrors;
		return std::nullopt;
	}

	std::size_t processedClauseCounter = 0;
	ProcessingError clauseParsingError;
	temporaryProcessingErrorContainer = optionalFoundErrors ? &clauseParsingError : nullptr;

	bool continueProcessing;
	do
	{
		++currProcessedLine;
		std::optional<ProblemDefinition::Clause> parsedClause = parseClauseDefinition(stream, problemDefinitionConfiguration->numVariables, *problemDefinition, temporaryProcessingErrorContainer);
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
			if (problemDefinition->propagate(unitPropagatedLiteral) != ProblemDefinition::Ok)
				recordError(currProcessedLine - 1, 0, "Error during unit propagation of literal " + std::to_string(unitPropagatedLiteral));
			else
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
		}
		else if (isClauseTautology(*parsedClause))
			recordError(currProcessedLine, 0, "Formula is expected to contain no tautologies");
		else
			problemDefinition->addClause(processedClauseCounter - 1, *parsedClause);
	} while (continueProcessing);

	if (processedClauseCounter != problemDefinitionConfiguration->numClauses)
		recordError(currProcessedLine, 0, "Expected formula to contain " + std::to_string(problemDefinitionConfiguration->numClauses) + " clauses but " + std::to_string(processedClauseCounter) + " were parsed");

	// TODO: Local variable elimination
	if (!foundErrorsDuringCurrentParsingAttempt)
		return std::move(problemDefinition);

	if (optionalFoundErrors)
		*optionalFoundErrors = foundErrors;
	return std::nullopt;
}

bool dimacs::DimacsParser::removeClausesSatisfiedByUnitPropagation(ProblemDefinition& problemDefinition, long literal)
{
	const LiteralOccurrenceLookup& literalOccurrenceLookup = problemDefinition.getLiteralOccurrenceLookup();
	const std::optional<std::vector<std::size_t>> clausesContainingLiteral = literalOccurrenceLookup[literal];
	if (!clausesContainingLiteral.has_value())
		return false;

	for (const std::size_t clauseIndex : *clausesContainingLiteral)
		if (!problemDefinition.removeClause(clauseIndex))
			return false;
	return true;
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
		foundErrors.emplace_back(line, column, errorText);
	//foundErrors.emplace_back("--line: " + std::to_string(line) + " col: " + std::to_string(column) + " | " + errorText);
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

inline std::optional<long> dimacs::DimacsParser::tryConvertStringToLong(const std::string_view& stringToConvert, ProcessingError* optionalFoundError)
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

inline std::optional<dimacs::DimacsParser::ProblemDefinitionConfiguration> dimacs::DimacsParser::processProblemDefinitionLine(std::basic_istream<char>& inputStream, ProcessingError* optionalFoundError)
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

inline std::optional<dimacs::ProblemDefinition::Clause> dimacs::DimacsParser::parseClauseDefinition(std::basic_istream<char>& inputStream, std::size_t numDefinedVariablesInCnf, const ProblemDefinition& variableValueLookupGateway, ProcessingError* optionalFoundErrors)
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
				if (currentValueOfVariable == ProblemDefinition::determineConflictingAssignmentForLiteral(*clauseLiteral))
				{
					if (optionalFoundErrors)
						*optionalFoundErrors = ProcessingError("Derived UNSAT of formula using current variable assignments");
					return std::nullopt;
				}
				if (currentValueOfVariable == ProblemDefinition::Unknown)
					clause.literals.emplace_back(*clauseLiteral);
			}
		}
	}

	if (!wasRequiredEndDelimiterDefined)
	{
		if (optionalFoundErrors)
			*optionalFoundErrors = ProcessingError("Clause must define literal 0 as its closing delimiter");
		return std::nullopt;
	}

	if (!doesCurrentVariableAssignmentSatisfyClause)
	{
		clause.sortLiterals();
		return clause;
	}
	return std::nullopt;
}

bool dimacs::DimacsParser::isClauseTautology(const dimacs::ProblemDefinition::Clause& clause) noexcept
{
	if (clause.literals.size() < 2)
		return false;

	std::unordered_set<long> literals;
	for (long literal : clause.literals)
	{
		if (literals.count(-literal))
			return true;
		literals.emplace(literal);
	}
	return false;
}