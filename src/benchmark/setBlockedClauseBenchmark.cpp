#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <dimacs/dimacsParser.hpp>
#include <optimizations/utils/clauseCandidateSelector.hpp>

#include "optimizations/setBlockedClauseElimination/baseSetBlockedClauseEliminator.hpp"
#include "optimizations/setBlockedClauseElimination/literalOccurrenceBlockingSetCandidateGenerator.hpp"
#include <optimizations/setBlockedClauseElimination/literalOccurrenceSetBlockedClauseEliminator.hpp>
#include <optimizations/setBlockedClauseElimination/avlIntervalTreeSetBlockedClauseEliminator.hpp>

#include "benchmark/commandLineArgumentParser.hpp"

const std::string clauseSelectionHeuristicCommandLineKey = "-clauseSelectionHeuristic";
const std::string clauseSelectionRngSeedCommandLineKey = "-clauseSelectionRngSeed";
const std::string blockingSetLiteralCandidateSelectionHeuristicCommandLineKey = "-blockingSetClauseLiteralCandiateSelectionHeuristic";
const std::string blockingSetLiteralCandidateSelectionRngSeedCommandLineKey = "-blockingSetClauseLiteralCandiateSelectionRngSeed";
const std::string blockingSetMinimumSizeRestrictionCommandLineKey = "-blockingSetMinimumSize";
const std::string blockingSetMaximumSizeRestrictionCommandLineKey = "-blockingSetMaximumSize";
const std::string blockingSetEliminatorSelectorCommandLineKey = "-blockingSetEliminator";
const std::string nClausesToConsiderCommandLineKey = "-nCandidates";
const std::string nClauseMatchesCommandLineKey = "-nMatches";
const std::string cnfFileCommandLineKey = "-cnf";
const std::string helpCommandLineKey = "--help";

/*
 * Prefer the usage of std::chrono::steady_clock instead of std::chrono::sytem_clock since the former cannot decrease (due to time zone changes, etc.) and is most suitable for measuring intervals according to (https://en.cppreference.com/w/cpp/chrono/steady_clock)
 */
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

TimePoint getCurrentTime()
{
	return std::chrono::steady_clock::now();
}

std::chrono::milliseconds getDurationBetweenTimestamps(const TimePoint endTimestamp, const TimePoint startTimestamp)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(endTimestamp - startTimestamp);
}


enum LiteralSelectionHeuristic
{
	Sequential,
	Random,
	MinimalClauseOverlap,
	MaximumClauseOverlap
};

enum BlockingSetEliminatorType {
	LiteralOccurrence,
	Avl,
};

struct ClauseCandidateGeneratorConfiguration
{
	std::size_t numCandidateClauses;
	std::size_t numCandidateClauseMatchesToSearchFor;
	std::optional<clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic> optionalCandidateSelectionHeuristic;
	std::optional<long> optionalRngSeed;
};

struct BlockingSetCandidateGeneratorConfiguration
{
	std::optional<long> optionalRngSeed;
	std::optional<setBlockedClauseElimination::BaseBlockingSetCandidateGenerator::CandidateSizeRestriction> optionalCandidateSizeRestriction;
	std::optional<LiteralSelectionHeuristic> optionalClauseLiteralSelectionHeuristic;
};

setBlockedClauseElimination::BaseBlockingSetCandidateGenerator::ptr initializeBlockingSetCandidateGenerator(const BlockingSetCandidateGeneratorConfiguration& blockingSetCandidateGeneratorConfiguration)
{
	if (blockingSetCandidateGeneratorConfiguration.optionalRngSeed.has_value())
		return setBlockedClauseElimination::LiteralOccurrenceBlockingSetCandidateGenerator::usingRandomLiteralSelectionHeuristic(*blockingSetCandidateGeneratorConfiguration.optionalRngSeed);
	if (blockingSetCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.has_value())
	{
		if (blockingSetCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.value() == LiteralSelectionHeuristic::MinimalClauseOverlap)
			return setBlockedClauseElimination::LiteralOccurrenceBlockingSetCandidateGenerator::usingMinimumClauseOverlapForLiteralSelection();
		if (blockingSetCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.value() == LiteralSelectionHeuristic::MaximumClauseOverlap)
			return setBlockedClauseElimination::LiteralOccurrenceBlockingSetCandidateGenerator::usingMaximumClauseOverlapForLiteralSelection();
	}
	return setBlockedClauseElimination::LiteralOccurrenceBlockingSetCandidateGenerator::usingSequentialLiteralSelectionHeuristic();
}

clauseCandidateSelection::ClauseCandidateSelector::ptr initializeClauseCandidateSelector(const dimacs::ProblemDefinition& parsedCnfFormula, const ClauseCandidateGeneratorConfiguration& clauseCandidateGeneratorConfiguration)
{
	if (clauseCandidateGeneratorConfiguration.optionalRngSeed.has_value())
		return clauseCandidateSelection::ClauseCandidateSelector::initUsingRandomCandidateSelection(clauseCandidateGeneratorConfiguration.numCandidateClauses, *clauseCandidateGeneratorConfiguration.optionalRngSeed);
	if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.has_value())
	{
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseOverlap)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMinimalClauseOverlapForCandidateSelection(parsedCnfFormula);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseOverlap)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMaximalClauseOverlapForCandidateSelection(parsedCnfFormula);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseLength)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMinimumClauseLenghtForCandidateSelection(parsedCnfFormula);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseLength)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMaximumClauseLengthForCandidateSelection(parsedCnfFormula);
	}
	return clauseCandidateSelection::ClauseCandidateSelector::initUsingSequentialCandidateSelection(clauseCandidateGeneratorConfiguration.numCandidateClauses);
}

ClauseCandidateGeneratorConfiguration generateClauseCandidateGeneratorConfigurationFromCommandLine(std::size_t numClausesOfFormulaAfterPreprocessing, const utils::CommandLineArgumentParser& commandLineArgumentParser)
{
	std::optional<long> optionalRngSeed;
	std::optional<clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic> optionalCandidateSelectionHeuristic;
	std::size_t nClausesToConsider;
	std::size_t nClauseMatchesToSearchFor;

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& nCandidateToConsiderCommandLineArgument = commandLineArgumentParser.getValueOfArgument(nClausesToConsiderCommandLineKey); nCandidateToConsiderCommandLineArgument.has_value()
		&& nCandidateToConsiderCommandLineArgument.value().wasFoundInCommandLineArgument)
	{
		if (!nCandidateToConsiderCommandLineArgument.value().tryGetArgumentValueAsInteger())
			throw std::invalid_argument("Number of clauses to consider as candidates was not defined as an integer value but was actually " + *nCandidateToConsiderCommandLineArgument.value().optionalArgumentValue);

		const int nCandidateToConsiderUserSpecifiedValue = *nCandidateToConsiderCommandLineArgument->tryGetArgumentValueAsInteger();
		if (nCandidateToConsiderUserSpecifiedValue < 0)
			throw std::invalid_argument("Number of candiate clauses to consider must be a positive integer");

		nClausesToConsider = std::min(numClausesOfFormulaAfterPreprocessing, static_cast<std::size_t>(nCandidateToConsiderUserSpecifiedValue));
	}
	else
	{
		nClausesToConsider = numClausesOfFormulaAfterPreprocessing;
	}

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& nCandidateMatchesToSearchForCommandLineArgument = commandLineArgumentParser.getValueOfArgument(nClauseMatchesCommandLineKey); nCandidateMatchesToSearchForCommandLineArgument.has_value())
	{
		if (!nCandidateMatchesToSearchForCommandLineArgument.value().wasFoundInCommandLineArgument)
			throw std::invalid_argument("Required value for number of clause matches to search for was not specified");

		if (!nCandidateMatchesToSearchForCommandLineArgument.value().tryGetArgumentValueAsInteger())
			throw std::invalid_argument("Number of clause matches to search for was not defined as an integer value but was actually " + *nCandidateMatchesToSearchForCommandLineArgument.value().optionalArgumentValue);

		const int nCandidateMatchesToSearchForUserSpecifiedValue = *nCandidateMatchesToSearchForCommandLineArgument->tryGetArgumentValueAsInteger();
		if (nCandidateMatchesToSearchForUserSpecifiedValue < 0)
			throw std::invalid_argument("Number of candiate clauses to consider must be a positive integer");

		nClauseMatchesToSearchFor = std::min(numClausesOfFormulaAfterPreprocessing, static_cast<std::size_t>(nCandidateMatchesToSearchForUserSpecifiedValue));
	}
	else
	{
		nClauseMatchesToSearchFor = numClausesOfFormulaAfterPreprocessing;
	}

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& clauseCandidateSelectionCommandLineArugment = commandLineArgumentParser.getValueOfArgument(clauseSelectionHeuristicCommandLineKey); clauseCandidateSelectionCommandLineArugment.has_value())
	{
		if (!clauseCandidateSelectionCommandLineArugment->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + clauseSelectionHeuristicCommandLineKey + " is missing");

		if (*clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "sequential")
		{
			optionalCandidateSelectionHeuristic = clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::Sequential;
		}
		else if (*clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "random")
		{
			const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& rngSeed = commandLineArgumentParser.getValueOfArgument(blockingSetLiteralCandidateSelectionRngSeedCommandLineKey);
			if (!rngSeed.has_value() || !rngSeed.value().wasFoundInCommandLineArgument)
				throw std::invalid_argument("Required rng seed for random clause literal selection for blocking set is missing");

			if (!rngSeed.value().tryGetArgumentValueAsInteger())
				throw std::invalid_argument("Could not convert specified rng seed " + *rngSeed.value().optionalArgumentValue + "for clause literal selection for blocking set check");

			optionalRngSeed = rngSeed->tryGetArgumentValueAsInteger();
			optionalCandidateSelectionHeuristic = clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::Random;
		}
		else if (*clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "minOverlap" || *clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "maxOverlap")
		{
			optionalCandidateSelectionHeuristic = *clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "minOverlap"
				? clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseOverlap
				: clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseOverlap;
		}
		else if (*clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "minLength" || *clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "maxLength")
		{
			optionalCandidateSelectionHeuristic = *clauseCandidateSelectionCommandLineArugment->optionalArgumentValue == "minLength"
				? clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseLength
				: clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseLength;
		}
		else
		{
			throw std::invalid_argument("Invalid value " + *clauseCandidateSelectionCommandLineArugment->optionalArgumentValue + "for blocking set clause literal selection heuristic");
		}
	}
	return { nClausesToConsider, nClauseMatchesToSearchFor, optionalCandidateSelectionHeuristic, optionalRngSeed };
}

BlockingSetCandidateGeneratorConfiguration generateBlockingSetCandidateGeneratorConfigurationFromCommandLine(const utils::CommandLineArgumentParser& commandLineArgumentParser)
{
	std::optional<long> optionalRngSeed;
	std::optional<setBlockedClauseElimination::BaseBlockingSetCandidateGenerator::CandidateSizeRestriction> optionalCandidateSizeRestriction;
	std::optional<LiteralSelectionHeuristic> optionalClauseLiteralSelectionHeuristic;

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& clauseLiteralSelectionHeuristicCommandLineArgument = commandLineArgumentParser.getValueOfArgument(blockingSetLiteralCandidateSelectionHeuristicCommandLineKey); clauseLiteralSelectionHeuristicCommandLineArgument.has_value()
		&& clauseLiteralSelectionHeuristicCommandLineArgument->wasFoundInCommandLineArgument)
	{
		if (!clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockingSetLiteralCandidateSelectionHeuristicCommandLineKey + " is missing");

		if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "sequential")
		{
			optionalClauseLiteralSelectionHeuristic = LiteralSelectionHeuristic::Sequential;
		}
		else if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "random")
		{
			const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& rngSeed = commandLineArgumentParser.getValueOfArgument(blockingSetLiteralCandidateSelectionRngSeedCommandLineKey);
			if (!rngSeed.has_value() || !rngSeed.value().wasFoundInCommandLineArgument)
				throw std::invalid_argument("Required rng seed for random clause literal selection for blocking set is missing");

			if (!rngSeed.value().tryGetArgumentValueAsInteger())
				throw std::invalid_argument("Could not convert specified rng seed " + *rngSeed.value().optionalArgumentValue + "for clause literal selection for blocking set check");

			optionalRngSeed = rngSeed->tryGetArgumentValueAsInteger();
			optionalClauseLiteralSelectionHeuristic = LiteralSelectionHeuristic::Random;
		}
		else if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "minOverlap")
		{
			optionalClauseLiteralSelectionHeuristic = LiteralSelectionHeuristic::MinimalClauseOverlap;
		}
		else if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "maxOverlap")
		{
			optionalClauseLiteralSelectionHeuristic = LiteralSelectionHeuristic::MaximumClauseOverlap;
		}
		else
		{
			throw std::invalid_argument("Invalid value " + *clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue + "for blocking set clause literal selection heuristic");
		}
	}

	std::optional<std::size_t> definedMinimumBlockingSetSize;
	std::optional<std::size_t> definedMaximumBlockingSetSize;
	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& blockingSetMinimumSizeRestrictionCommandLineArgument = commandLineArgumentParser.getValueOfArgument(blockingSetMinimumSizeRestrictionCommandLineKey); blockingSetMinimumSizeRestrictionCommandLineArgument.has_value()
		&& blockingSetMinimumSizeRestrictionCommandLineArgument->wasFoundInCommandLineArgument)
	{
		if (!blockingSetMinimumSizeRestrictionCommandLineArgument->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockingSetMinimumSizeRestrictionCommandLineKey + " is missing");

		definedMinimumBlockingSetSize = blockingSetMinimumSizeRestrictionCommandLineArgument->tryGetArgumentValueAsInteger();
		if (!definedMinimumBlockingSetSize.has_value() || blockingSetMinimumSizeRestrictionCommandLineArgument->tryGetArgumentValueAsInteger().value() < 0)
			throw std::invalid_argument("Expected positive integer value for command line argument " + blockingSetMinimumSizeRestrictionCommandLineKey + " but was actually " + blockingSetMinimumSizeRestrictionCommandLineArgument->optionalArgumentValue.value());
	}

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& blockingSetMaximumSizeRestrictionCommandLineArgument = commandLineArgumentParser.getValueOfArgument(blockingSetMaximumSizeRestrictionCommandLineKey); blockingSetMaximumSizeRestrictionCommandLineArgument.has_value()
		&& blockingSetMaximumSizeRestrictionCommandLineArgument->wasFoundInCommandLineArgument)
	{
		if (!blockingSetMaximumSizeRestrictionCommandLineArgument->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockingSetMinimumSizeRestrictionCommandLineKey + " is missing");

		definedMaximumBlockingSetSize = blockingSetMaximumSizeRestrictionCommandLineArgument->tryGetArgumentValueAsInteger();
		if (!definedMaximumBlockingSetSize.has_value() || blockingSetMaximumSizeRestrictionCommandLineArgument->tryGetArgumentValueAsInteger().value() < 0)
			throw std::invalid_argument("Expected positive integer value for command line argument " + blockingSetMinimumSizeRestrictionCommandLineKey + " but was actually " + blockingSetMaximumSizeRestrictionCommandLineArgument->optionalArgumentValue.value());
	}

	if (definedMinimumBlockingSetSize.has_value() ^ definedMaximumBlockingSetSize.has_value())
		throw std::invalid_argument("When defining blocking set size restrictions both the minimum as well as the maximum allowed size needs to be specified");
	if (definedMinimumBlockingSetSize.has_value())
		optionalCandidateSizeRestriction = setBlockedClauseElimination::BaseBlockingSetCandidateGenerator::CandidateSizeRestriction({ *definedMinimumBlockingSetSize, *definedMaximumBlockingSetSize });

	return { optionalRngSeed, optionalCandidateSizeRestriction, optionalClauseLiteralSelectionHeuristic };
}

BlockingSetEliminatorType determineTypeOfBlockingSetEliminatorFromCommandLine(const utils::CommandLineArgumentParser& commandLineArgumentParser) {
	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& userProvidedBlockingSetEliminatorType = commandLineArgumentParser.getValueOfArgument(blockingSetEliminatorSelectorCommandLineKey); userProvidedBlockingSetEliminatorType.has_value()
		&& userProvidedBlockingSetEliminatorType->wasFoundInCommandLineArgument)
	{
		if (!userProvidedBlockingSetEliminatorType->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockingSetEliminatorSelectorCommandLineKey + " is missing");

		if (*userProvidedBlockingSetEliminatorType->optionalArgumentValue == "literalOccurrence")
			return BlockingSetEliminatorType::LiteralOccurrence;
		if (*userProvidedBlockingSetEliminatorType->optionalArgumentValue == "avl")
			return BlockingSetEliminatorType::Avl;

		throw std::invalid_argument("Invalid value " + *userProvidedBlockingSetEliminatorType->optionalArgumentValue + " for comand line argument " + blockingSetEliminatorSelectorCommandLineKey);
	}
	throw std::invalid_argument("Required command line argument " + blockingSetEliminatorSelectorCommandLineKey + " was not found");
}

int main(int argc, char* argv[])
{
	std::unique_ptr<dimacs::DimacsParser> dimacsParser = std::make_unique<dimacs::DimacsParser>(dimacs::DimacsParser::ParserConfiguration({ false, true }));
	if (!dimacsParser)
	{
		std::cerr << "Failed to allocate resources for dimacs parser\n";
		return EXIT_FAILURE;
	}

	auto commandLineArgumentParser = utils::CommandLineArgumentParser();
	commandLineArgumentParser.registerCommandLineArgument(cnfFileCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument());
	commandLineArgumentParser.registerCommandLineArgument(clauseSelectionHeuristicCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(clauseSelectionRngSeedCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockingSetLiteralCandidateSelectionHeuristicCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockingSetLiteralCandidateSelectionRngSeedCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(nClausesToConsiderCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockingSetMinimumSizeRestrictionCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockingSetMaximumSizeRestrictionCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(helpCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createValueLessArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockingSetEliminatorSelectorCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument());

	try
	{
		commandLineArgumentParser.processCommandLineArguments(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Failed to parse command line arguments, reason: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& helpCommandRegistation = commandLineArgumentParser.getValueOfArgument(helpCommandLineKey); helpCommandRegistation.has_value() && helpCommandRegistation->wasFoundInCommandLineArgument)
	{
		std::cout << commandLineArgumentParser << "\n";
		return EXIT_SUCCESS;
	}

	const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& cnfFileCommandRegistration = commandLineArgumentParser.getValueOfArgument(cnfFileCommandLineKey);
	if (!cnfFileCommandRegistration.has_value() || !cnfFileCommandRegistration->wasFoundInCommandLineArgument || !cnfFileCommandRegistration->optionalArgumentValue.has_value())
		std::cerr << "Required cnf file path was no defined" << "\n";

	//const std::string dimacsSatFormulaFile = argv[1];
	const std::string dimacsSatFormulaFile = cnfFileCommandRegistration->optionalArgumentValue.value();

	std::cout << "=== START - PROCESSING CNF ===\n";
	const TimePoint dimacsFormulaParsingStartTime = getCurrentTime();
	const dimacs::DimacsParser::ParseResult parsingResult = dimacsParser->readProblemFromFile(dimacsSatFormulaFile);
	const TimePoint dimacsFormulaParsingEndtime = getCurrentTime();

	const std::chrono::milliseconds dimacsFormulaParsingDuration = getDurationBetweenTimestamps(dimacsFormulaParsingEndtime, dimacsFormulaParsingStartTime);
	std::cout << "Duration for processing of cnf formula: " + std::to_string(dimacsFormulaParsingDuration.count()) + "ms\n";
	std::cout << "=== END - PROCESSING CNF ===\n";

	if (parsingResult.determinedAnyErrors)
	{
		std::ostringstream out;
		for (const auto& processingError : parsingResult.errors)
			out << processingError << "\r\n";

		if (parsingResult.errors.size() > 1)
			out << parsingResult.errors.back();

		std::cerr << out.str() << "\n";
		return EXIT_FAILURE;
	}
	if (!parsingResult.formula.has_value())
		return EXIT_FAILURE;

	std::cout << "Parsing of SAT formula @ " + dimacsSatFormulaFile + " OK\n";
	dimacs::ProblemDefinition::ptr cnfFormula = parsingResult.formula.value();

	ClauseCandidateGeneratorConfiguration clauseCandidateGeneratorConfiguration;
	BlockingSetCandidateGeneratorConfiguration blockingSetCandidateGeneratorConfiguration;

	try
	{
		clauseCandidateGeneratorConfiguration = generateClauseCandidateGeneratorConfigurationFromCommandLine(cnfFormula->getNumClausesAfterOptimizations(), commandLineArgumentParser);
		blockingSetCandidateGeneratorConfiguration = generateBlockingSetCandidateGeneratorConfigurationFromCommandLine(commandLineArgumentParser);
	}
	catch (const std::invalid_argument& ex)
	{
		std::cerr << "Validation of command line arguments failed, reason: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "=== START - CLAUSE CANDIDATE GENERATION INITIALIZATION ===\n";
	const TimePoint clauseCandidateGeneratorInitStartTime = getCurrentTime();
	clauseCandidateSelection::ClauseCandidateSelector::ptr clauseCandidateSelector = initializeClauseCandidateSelector(*cnfFormula, clauseCandidateGeneratorConfiguration);
	const TimePoint clauseCandidateGeneratorInitEndTime = getCurrentTime();

	const std::chrono::milliseconds clauseCandidateGeneratorInitDuration = getDurationBetweenTimestamps(clauseCandidateGeneratorInitEndTime, clauseCandidateGeneratorInitStartTime);
	std::cout << "Duration for generation of clause candidate generator duration: " + std::to_string(clauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "=== END - CLAUSE CANDIDATE GENERATOR INITIALIZATION ===\n";

	std::cout << "=== START - BLOCKING SET CANDIDATE GENERATOR INITALIZATION ===\n";
	const TimePoint blockingSetCandidateGeneratorInitStartTime = getCurrentTime();
	setBlockedClauseElimination::BaseBlockingSetCandidateGenerator::ptr blockingSetCandidateGenerator = initializeBlockingSetCandidateGenerator(blockingSetCandidateGeneratorConfiguration);
	const TimePoint blockingSetCandidateGeneratorInitEndTime = getCurrentTime();

	const std::chrono::milliseconds blockingSetCandidateGeneratorInitDuration = getDurationBetweenTimestamps(blockingSetCandidateGeneratorInitEndTime, blockingSetCandidateGeneratorInitStartTime);
	std::cout << "Duration for generation of clause candidate generator duration: " + std::to_string(clauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "=== END - BLOCKING SET CANDIDATE GENERATOR INITALIZATION ===\n";

	std::cout << "=== START - BLOCKING SET ELIMINATOR INITIALIZATION ===\n";
	const BlockingSetEliminatorType blockingSetEliminatorType = determineTypeOfBlockingSetEliminatorFromCommandLine(commandLineArgumentParser);
	const TimePoint blockingSetEliminatorInitStartTime = getCurrentTime();

	std::unique_ptr<setBlockedClauseElimination::BaseSetBlockedClauseEliminator> blockingSetEliminator;
	if (blockingSetEliminatorType == BlockingSetEliminatorType::LiteralOccurrence)
		blockingSetEliminator = std::make_unique<setBlockedClauseElimination::LiteralOccurrenceSetBlockedClauseEliminator>(cnfFormula);
	else {
		auto avlTreeBlockingSetEliminator = std::make_unique<setBlockedClauseElimination::AvlIntervalTreeSetBlockedClauseEliminator>(cnfFormula);
		if (!avlTreeBlockingSetEliminator->initializeAvlTree()) {
			std::cout << "Failed to construct avl interval tree\n";
			return EXIT_FAILURE;
		}
		blockingSetEliminator = std::move(avlTreeBlockingSetEliminator);
	}
	const TimePoint blockingSetEliminatorInitEndTime = getCurrentTime();
	const std::chrono::milliseconds blockingSetEliminatorInitDuration = getDurationBetweenTimestamps(blockingSetEliminatorInitEndTime, blockingSetEliminatorInitStartTime);
	std::cout << "Duration for initialization of the blocking set eliminator " + std::to_string(blockingSetEliminatorInitDuration.count()) + "ms\n";
	std::cout << "=== END - BLOCKING SET ELIMINATOR INITIALIZATION ===\n";

	std::cout << "=== START - BLOCKING SET CANDIDATE SEARCH ===\n";
	std::cout << "Search for set blocked clauses will stop when " << std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauseMatchesToSearchFor) << " blocked clauses were found...\n";
	std::cout << "Search for set blocked clauses will stop when " << std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauses) << " candidates where considered...\n";

	const std::vector<std::size_t>& identifiersOfClausesToCheck = cnfFormula->getIdentifiersOfClauses();

	constexpr std::size_t percentageThreshold = 5;
	std::size_t numClausesToProcessUntilPercentageThresholdIsReached = clauseCandidateGeneratorConfiguration.numCandidateClauses / (100 / percentageThreshold);
	if (numClausesToProcessUntilPercentageThresholdIsReached == 0)
		++numClausesToProcessUntilPercentageThresholdIsReached;

	std::size_t percentThresholdReachedCounter = 0;
	std::size_t remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;

	std::vector<std::size_t> identifiersOfSetBlockedClauses;
	std::size_t numCandiatesConsidered = 0;

	std::chrono::milliseconds totalBenchmarkExecutionTime = dimacsFormulaParsingDuration + clauseCandidateGeneratorInitDuration + blockingSetCandidateGeneratorInitDuration + blockingSetEliminatorInitDuration;
	std::chrono::milliseconds setBlockedClauseCheckDuration = std::chrono::milliseconds(0);

	std::cout << "Progress will be logged every time " << std::to_string(numClausesToProcessUntilPercentageThresholdIsReached) << " candidates where processed...\n\n";
	for (const std::size_t clauseIdentifier : identifiersOfClausesToCheck)
	{
		if (identifiersOfSetBlockedClauses.size() >= clauseCandidateGeneratorConfiguration.numCandidateClauseMatchesToSearchFor)
		{
			std::cout << "Reached required number of set blocked clauses found, will stop search...\n";
			break;
		}
		if (numCandiatesConsidered > clauseCandidateGeneratorConfiguration.numCandidateClauses)
		{
			std::cout << "Reached required number of clause candidates, will stop search...\n";
			break;
		}
		++numCandiatesConsidered;

		const TimePoint blockingSetCheckStartTime = getCurrentTime();
		if (const std::optional<setBlockedClauseElimination::BaseSetBlockedClauseEliminator::FoundBlockingSet>& foundBlockingSet = blockingSetEliminator->determineBlockingSet(clauseIdentifier, *blockingSetCandidateGenerator, blockingSetCandidateGeneratorConfiguration.optionalCandidateSizeRestriction); foundBlockingSet.has_value())
			identifiersOfSetBlockedClauses.emplace_back(clauseIdentifier);

		const TimePoint blockingSetCheckEndTime = getCurrentTime();
		totalBenchmarkExecutionTime += getDurationBetweenTimestamps(blockingSetCheckEndTime, blockingSetCheckStartTime);
		setBlockedClauseCheckDuration += getDurationBetweenTimestamps(blockingSetCheckEndTime, blockingSetCheckStartTime);

		--remainingNumClausesForPercentageThreshold;
		if (!remainingNumClausesForPercentageThreshold)
		{
			remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;
			++percentThresholdReachedCounter;
			std::cout << "Handled [" << std::to_string(percentThresholdReachedCounter * percentageThreshold) << "%] of all candidate clauses, current duration of blocked set check: " << std::to_string(setBlockedClauseCheckDuration.count()) << " current benchmark duration : " << std::to_string(totalBenchmarkExecutionTime.count()) + "ms\n";
		}
	}
	std::cout << "=== END - BLOCKING SET CANDIDATE SEARCH ===\n";

	std::cout << "=== BEGIN - VERIFICATION OF RESULTS ===\n";
	const TimePoint resultVerificationStartTime = getCurrentTime();

	std::cout << "SKIPPED FOR NOW...\n";

	const TimePoint resultVerificationEndTime = getCurrentTime();
	const std::chrono::milliseconds resultVerificationDuration = getDurationBetweenTimestamps(resultVerificationEndTime, resultVerificationStartTime);
	std::cout << "=== BEGIN - VERIFICATION OF RESULTS ===\n";

	std::cout << "=== RESULTS ===\n";
	std::cout << std::to_string(identifiersOfSetBlockedClauses.size()) + " clauses out of " + std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauses) + " were set blocked!\n";

	std::cout << "Duration for processing of DIMACS formula: " + std::to_string(dimacsFormulaParsingDuration.count()) + "ms\n";
	std::cout << "Duration for initialization of blocking set eliminator: " + std::to_string(blockingSetCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "SBCE check duration: " + std::to_string(setBlockedClauseCheckDuration.count()) + "ms\n";
	std::cout << "SBCE result verification duration: " + std::to_string(resultVerificationDuration.count()) + "ms\n";
	std::cout << "TOTAL: " + std::to_string(totalBenchmarkExecutionTime.count()) + "ms\n";
	return EXIT_SUCCESS;
}
