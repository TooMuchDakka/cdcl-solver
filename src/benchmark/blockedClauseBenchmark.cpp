#include "benchmark/commandLineArgumentParser.hpp"
#include <dimacs/dimacsParser.hpp>

#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"
#include "optimizations/blockedClauseElimination/baseBlockedClauseEliminator.hpp"
#include "optimizations/blockedClauseElimination/blockingLiteralGenerator.hpp"
#include "optimizations/blockedClauseElimination/literalOccurrenceBlockedClauseEliminator.hpp"
#include "optimizations/utils/clauseCandidateSelector.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

const std::string clauseSelectionHeuristicCommandLineKey = "-clauseSelectionHeuristic";
const std::string clauseSelectionRngSeedCommandLineKey = "-clauseSelectionRngSeed";
const std::string blockedClauseLiteralCandidateSelectionHeuristicCommandLineKey = "-blockedClauseLiteralCandiateSelectionHeuristic";
const std::string blockedClauseLiteralCandidateSelectionRngSeedCommandLineKey = "-blockedClauseLiteralCandiateSelectionRngSeed";
const std::string blockedClauseEliminatorSelectorCommandLineKey = "-blockedClauseEliminator";
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

enum BlockedClauseEliminatorType {
	LiteralOccurrence,
	Avl,
};

struct ClauseCandidateGeneratorConfiguration
{
	std::size_t numCandidateClauses;
	std::size_t numCandidateClauseMatchesToSearchFor;
	std::optional<clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic> optionalCandidateSelectionHeuristic;
	std::optional<long> optionalRngSeed;
	std::optional<clauseCandidateSelection::ClauseCandidateSelector::ClauseLengthRestriction> optionalClauseLengthRestriction;
};

struct BlockedClauseCandidateGeneratorConfiguration
{
	std::optional<long> optionalRngSeed;
	std::optional<LiteralSelectionHeuristic> optionalClauseLiteralSelectionHeuristic;
};

blockedClauseElimination::BlockingLiteralGenerator::ptr initializeBlockedClauseCandidateGenerator(const BlockedClauseCandidateGeneratorConfiguration& blockedClauseCandidateGeneratorConfiguration)
{
	if (blockedClauseCandidateGeneratorConfiguration.optionalRngSeed.has_value())
		return blockedClauseElimination::BlockingLiteralGenerator::usingRandomLiteralSelectionHeuristic(*blockedClauseCandidateGeneratorConfiguration.optionalRngSeed);
	if (blockedClauseCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.has_value())
	{
		if (blockedClauseCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.value() == LiteralSelectionHeuristic::MinimalClauseOverlap)
			return blockedClauseElimination::BlockingLiteralGenerator::usingMinimumClauseOverlapForLiteralSelection();
		if (blockedClauseCandidateGeneratorConfiguration.optionalClauseLiteralSelectionHeuristic.value() == LiteralSelectionHeuristic::MaximumClauseOverlap)
			return blockedClauseElimination::BlockingLiteralGenerator::usingMaximumClauseOverlapForLiteralSelection();
	}
	return blockedClauseElimination::BlockingLiteralGenerator::usingSequentialLiteralSelectionHeuristic();
}

clauseCandidateSelection::ClauseCandidateSelector::ptr initializeClauseCandidateSelector(const dimacs::ProblemDefinition& parsedCnfFormula, const ClauseCandidateGeneratorConfiguration& clauseCandidateGeneratorConfiguration)
{
	if (clauseCandidateGeneratorConfiguration.optionalRngSeed.has_value())
		return clauseCandidateSelection::ClauseCandidateSelector::initUsingRandomCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.numCandidateClauses, *clauseCandidateGeneratorConfiguration.optionalRngSeed, clauseCandidateGeneratorConfiguration.optionalClauseLengthRestriction);
	if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.has_value())
	{
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseOverlap)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMinimalClauseOverlapForCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.optionalClauseLengthRestriction);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseOverlap)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMaximalClauseOverlapForCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.optionalClauseLengthRestriction);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MinimumClauseLength)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMinimumClauseLenghtForCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.optionalClauseLengthRestriction);
		if (clauseCandidateGeneratorConfiguration.optionalCandidateSelectionHeuristic.value() == clauseCandidateSelection::ClauseCandidateSelector::CandidateSelectionHeuristic::MaximumClauseLength)
			return clauseCandidateSelection::ClauseCandidateSelector::initUsingMaximumClauseLengthForCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.optionalClauseLengthRestriction);
	}
	return clauseCandidateSelection::ClauseCandidateSelector::initUsingSequentialCandidateSelection(parsedCnfFormula, clauseCandidateGeneratorConfiguration.numCandidateClauses, std::nullopt);
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
			const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& rngSeed = commandLineArgumentParser.getValueOfArgument(blockedClauseLiteralCandidateSelectionRngSeedCommandLineKey);
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

BlockedClauseCandidateGeneratorConfiguration generateBlockedClauseCandidateGeneratorConfigurationFromCommandLine(const utils::CommandLineArgumentParser& commandLineArgumentParser)
{
	std::optional<long> optionalRngSeed;
	std::optional<LiteralSelectionHeuristic> optionalClauseLiteralSelectionHeuristic;

	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& clauseLiteralSelectionHeuristicCommandLineArgument = commandLineArgumentParser.getValueOfArgument(blockedClauseLiteralCandidateSelectionHeuristicCommandLineKey); clauseLiteralSelectionHeuristicCommandLineArgument.has_value()
		&& clauseLiteralSelectionHeuristicCommandLineArgument->wasFoundInCommandLineArgument)
	{
		if (!clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockedClauseLiteralCandidateSelectionHeuristicCommandLineKey + " is missing");

		if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "sequential")
		{
			optionalClauseLiteralSelectionHeuristic = LiteralSelectionHeuristic::Sequential;
		}
		else if (*clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue == "random")
		{
			const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& rngSeed = commandLineArgumentParser.getValueOfArgument(blockedClauseLiteralCandidateSelectionRngSeedCommandLineKey);
			if (!rngSeed.has_value() || !rngSeed.value().wasFoundInCommandLineArgument)
				throw std::invalid_argument("Required rng seed for random clause literal selection heuristic is missing");

			if (!rngSeed.value().tryGetArgumentValueAsInteger())
				throw std::invalid_argument("Could not convert specified rng seed " + *rngSeed.value().optionalArgumentValue + "for random clause literal selection heuristic");

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
			throw std::invalid_argument("Invalid value " + *clauseLiteralSelectionHeuristicCommandLineArgument->optionalArgumentValue + "for blocked clause literal selection heuristic");
		}
	}
	return { optionalRngSeed, optionalClauseLiteralSelectionHeuristic };
}

BlockedClauseEliminatorType determineTypeOfBlockingSetEliminatorFromCommandLine(const utils::CommandLineArgumentParser& commandLineArgumentParser) {
	if (const std::optional<utils::CommandLineArgumentParser::CommandLineArgumentRegistration>& userProvidedBlockingSetEliminatorType = commandLineArgumentParser.getValueOfArgument(blockedClauseEliminatorSelectorCommandLineKey); userProvidedBlockingSetEliminatorType.has_value()
		&& userProvidedBlockingSetEliminatorType->wasFoundInCommandLineArgument)
	{
		if (!userProvidedBlockingSetEliminatorType->optionalArgumentValue.has_value())
			throw std::invalid_argument("Required value for command line argument " + blockedClauseEliminatorSelectorCommandLineKey + " is missing");

		if (*userProvidedBlockingSetEliminatorType->optionalArgumentValue == "literalOccurrence")
			return BlockedClauseEliminatorType::LiteralOccurrence;
		if (*userProvidedBlockingSetEliminatorType->optionalArgumentValue == "avl")
			return BlockedClauseEliminatorType::Avl;

		throw std::invalid_argument("Invalid value " + *userProvidedBlockingSetEliminatorType->optionalArgumentValue + " for comand line argument " + blockedClauseEliminatorSelectorCommandLineKey);
	}
	throw std::invalid_argument("Required command line argument " + blockedClauseEliminatorSelectorCommandLineKey + " was not found");
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
	commandLineArgumentParser.registerCommandLineArgument(blockedClauseLiteralCandidateSelectionHeuristicCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockedClauseLiteralCandidateSelectionRngSeedCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(nClausesToConsiderCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createIntegerArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(helpCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createValueLessArgument().asOptionalArgument());
	commandLineArgumentParser.registerCommandLineArgument(blockedClauseEliminatorSelectorCommandLineKey, utils::CommandLineArgumentParser::CommandLineArgumentRegistration::createStringArgument());

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

	std::cout << "=== START - USER PROVIDED COMMAND LINE ARGUMENTS ===" << "\n";
	std::cout << commandLineArgumentParser << "\n";
	std::cout << "=== END - USER PROVIDED COMMAND LINE ARGUMENTS ===" << "\n";

	//const std::string dimacsSatFormulaFile = argv[1];
	const std::string dimacsSatFormulaFile = cnfFileCommandRegistration->optionalArgumentValue.value();

	std::cout << "=== START - PROCESSING CNF ===\n";
	const TimePoint dimacsFormulaParsingStartTime = getCurrentTime();
	const dimacs::DimacsParser::ParseResult parsingResult = dimacsParser->readProblemFromFile(dimacsSatFormulaFile);
	const TimePoint dimacsFormulaParsingEndtime = getCurrentTime();

	const std::chrono::milliseconds dimacsFormulaParsingDuration = getDurationBetweenTimestamps(dimacsFormulaParsingEndtime, dimacsFormulaParsingStartTime);
	std::cout << "Duration for processing of cnf formula: " + std::to_string(dimacsFormulaParsingDuration.count()) + "ms\n";
	std::cout << "=== END - PROCESSING CNF ===\n\n";

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
	BlockedClauseCandidateGeneratorConfiguration blockedClauseCandidateGeneratorConfiguration;

	try
	{
		clauseCandidateGeneratorConfiguration = generateClauseCandidateGeneratorConfigurationFromCommandLine(cnfFormula->getNumClausesAfterOptimizations(), commandLineArgumentParser);
		blockedClauseCandidateGeneratorConfiguration = generateBlockedClauseCandidateGeneratorConfigurationFromCommandLine(commandLineArgumentParser);
	}
	catch (const std::invalid_argument& ex)
	{
		std::cerr << "Validation of command line arguments failed, reason: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "=== START - CLAUSE CANDIDATE GENERATION INITIALIZATION ===\n";
	const TimePoint clauseCandidateGeneratorInitStartTime = getCurrentTime();
	clauseCandidateSelection::ClauseCandidateSelector::ptr clauseCandidateSelector = initializeClauseCandidateSelector(*cnfFormula, clauseCandidateGeneratorConfiguration);
	std::vector<std::size_t> indicesOfCandidateClauses(clauseCandidateGeneratorConfiguration.numCandidateClauses, 0);
	for (std::size_t i = 0; i < indicesOfCandidateClauses.size(); ++i)
	{
		const std::optional<std::size_t> candidateClauseIndex = clauseCandidateSelector->selectNextCandidate();
		if (!candidateClauseIndex.has_value())
		{
			std::cerr << "Failed to generate candiate #" << std::to_string(i) << "\n";
			return EXIT_FAILURE;
		}
		indicesOfCandidateClauses[i] = *candidateClauseIndex;
	}
	const TimePoint clauseCandidateGeneratorInitEndTime = getCurrentTime();

	const std::chrono::milliseconds clauseCandidateGeneratorInitDuration = getDurationBetweenTimestamps(clauseCandidateGeneratorInitEndTime, clauseCandidateGeneratorInitStartTime);
	std::cout << "Candidate generator selected " + std::to_string(indicesOfCandidateClauses.size()) + " candidates when " + std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauses) + " were requested\n";
	std::cout << "Duration for generation of clause candidate generator duration: " + std::to_string(clauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "=== END - CLAUSE CANDIDATE GENERATOR INITIALIZATION ===\n\n";

	std::cout << "=== START - BLOCKED CLAUSE LITERAL CANDIDATE GENERATOR INITALIZATION ===\n";
	const TimePoint blockedClauseCandidateGeneratorInitStartTime = getCurrentTime();
	blockedClauseElimination::BlockingLiteralGenerator::ptr blockingLiteralCandiateGenerator = initializeBlockedClauseCandidateGenerator(blockedClauseCandidateGeneratorConfiguration);
	const TimePoint blockedClauseCandidateGeneratorInitEndTime = getCurrentTime();

	const std::chrono::milliseconds blockedClauseCandidateGeneratorInitDuration = getDurationBetweenTimestamps(blockedClauseCandidateGeneratorInitEndTime, blockedClauseCandidateGeneratorInitStartTime);
	std::cout << "Duration for generation of clause literal candidate generator duration: " + std::to_string(blockedClauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "=== END - BLOCKED CLAUSE LITERAL CANDIDATE GENERATOR INITALIZATION ===\n\n";

	std::cout << "=== START - BLOCKED CLAUSE ELIMINATOR INITIALIZATION ===\n";
	const BlockedClauseEliminatorType blockedClauseEliminatorType = determineTypeOfBlockingSetEliminatorFromCommandLine(commandLineArgumentParser);
	const TimePoint blockedClauseEliminatorInitStartTime = getCurrentTime();

	std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator;
	if (blockedClauseEliminatorType == BlockedClauseEliminatorType::LiteralOccurrence)
		blockedClauseEliminator = std::make_unique<blockedClauseElimination::LiteralOccurrenceBlockedClauseEliminator>(cnfFormula);
	else {
		auto avlTreeBlockedClauseEliminator = std::make_unique<blockedClauseElimination::AvlIntervalTreeBlockedClauseEliminator>(cnfFormula);
		if (!avlTreeBlockedClauseEliminator->initializeAvlTree()) {
			std::cout << "Failed to construct avl interval tree\n";
			return EXIT_FAILURE;
		}
		blockedClauseEliminator = std::move(avlTreeBlockedClauseEliminator);
	}
	const TimePoint blockedClauseEliminatorInitEndTime = getCurrentTime();
	const std::chrono::milliseconds blockedClauseEliminatorInitDuration = getDurationBetweenTimestamps(blockedClauseEliminatorInitEndTime, blockedClauseEliminatorInitStartTime);
	std::cout << "Duration for initialization of the blocked clause eliminator " + std::to_string(blockedClauseEliminatorInitDuration.count()) + "ms\n";
	std::cout << "=== END - BLOCKED CLAUSE ELIMINATOR INITIALIZATION ===\n\n";

	std::cout << "=== START - BLOCKED CLAUSE CANDIDATE SEARCH ===\n";
	std::cout << "User requested that search for blocked clauses stops when " << std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauseMatchesToSearchFor) << " blocked clauses were found...\n";
	std::cout << "User requested that search for blocked clauses stops when " << std::to_string(clauseCandidateGeneratorConfiguration.numCandidateClauses) << " candidates where considered...\n";

	const std::size_t numCandidatesToConsider = indicesOfCandidateClauses.size();
	const std::size_t numBlockedClausesToSearchFor = std::min(clauseCandidateGeneratorConfiguration.numCandidateClauseMatchesToSearchFor, numCandidatesToConsider);
	std::cout << "Search for blocked clauses stops when " << std::to_string(numBlockedClausesToSearchFor) << " blocked clauses were found...\n";
	std::cout << "Search for blocked clauses stops when " << std::to_string(numCandidatesToConsider) << " candidates where considered...\n";

	constexpr std::size_t percentageThreshold = 5;
	std::size_t numClausesToProcessUntilPercentageThresholdIsReached = numCandidatesToConsider / (100 / percentageThreshold);
	if (numClausesToProcessUntilPercentageThresholdIsReached == 0)
		++numClausesToProcessUntilPercentageThresholdIsReached;

	std::size_t percentThresholdReachedCounter = 0;
	std::size_t remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;

	std::vector<std::size_t> identifiersOfBlockedClauses;
	std::size_t numCandidatesConsidered = 0;

	std::chrono::milliseconds totalBenchmarkExecutionTime = dimacsFormulaParsingDuration + clauseCandidateGeneratorInitDuration + blockedClauseCandidateGeneratorInitDuration + blockedClauseEliminatorInitDuration;
	std::chrono::milliseconds setBlockedClauseCheckDuration = std::chrono::milliseconds(0);

	std::cout << "Progress will be logged every time " << std::to_string(numClausesToProcessUntilPercentageThresholdIsReached) << " candidates where processed...\n\n";
	for (const std::size_t clauseIdentifier : indicesOfCandidateClauses)
	{
		if (identifiersOfBlockedClauses.size() >= numBlockedClausesToSearchFor)
		{
			std::cout << "Reached required number of blocked clauses found, will stop search...\n";
			break;
		}
		if (numCandidatesConsidered > numCandidatesToConsider)
		{
			std::cout << "Reached required number of clause candidates, will stop search...\n";
			break;
		}
		++numCandidatesConsidered;

		const TimePoint blockedClauseCheckStartTime = getCurrentTime();
		if (blockedClauseEliminator->determineBlockingLiteralOfClause(clauseIdentifier, *blockingLiteralCandiateGenerator).has_value())
			identifiersOfBlockedClauses.emplace_back(clauseIdentifier);

		const TimePoint blockedClauseCheckEndTime = getCurrentTime();
		totalBenchmarkExecutionTime += getDurationBetweenTimestamps(blockedClauseCheckEndTime, blockedClauseCheckStartTime);
		setBlockedClauseCheckDuration += getDurationBetweenTimestamps(blockedClauseCheckEndTime, blockedClauseCheckStartTime);

		--remainingNumClausesForPercentageThreshold;
		if (!remainingNumClausesForPercentageThreshold)
		{
			remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;
			++percentThresholdReachedCounter;
			std::cout << "Handled [" << std::to_string(percentThresholdReachedCounter * percentageThreshold) << "%] of all candidate clauses, current duration of blocked clauses check: " << std::to_string(setBlockedClauseCheckDuration.count()) << "ms | current benchmark duration : " << std::to_string(totalBenchmarkExecutionTime.count()) + "ms\n";
		}
	}
	std::cout << "=== END - BLOCKED CLAUSE CANDIDATE SEARCH ===\n\n";

	std::cout << "=== BEGIN - VERIFICATION OF RESULTS ===\n";
	const TimePoint resultVerificationStartTime = getCurrentTime();

	std::cout << "SKIPPED FOR NOW...\n";

	const TimePoint resultVerificationEndTime = getCurrentTime();
	const std::chrono::milliseconds resultVerificationDuration = getDurationBetweenTimestamps(resultVerificationEndTime, resultVerificationStartTime);
	std::cout << "=== END - VERIFICATION OF RESULTS ===\n\n";

	std::cout << "=== RESULTS ===\n";
	std::cout << std::to_string(identifiersOfBlockedClauses.size()) + " clauses out of " + std::to_string(numCandidatesToConsider) + " were blocked!\n\n";

	std::cout << "Duration for processing of DIMACS formula: " + std::to_string(dimacsFormulaParsingDuration.count()) + "ms\n";
	std::cout << "Duration for initialization of candidate clause generator: " + std::to_string(clauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "Duration for initialization of blocked clause eliminator: " + std::to_string(blockedClauseCandidateGeneratorInitDuration.count()) + "ms\n";
	std::cout << "BCE check duration: " + std::to_string(setBlockedClauseCheckDuration.count()) + "ms\n";
	std::cout << "BCE result verification duration: " + std::to_string(resultVerificationDuration.count()) + "ms\n";
	std::cout << "TOTAL: " + std::to_string(totalBenchmarkExecutionTime.count()) + "ms\n";
	return EXIT_SUCCESS;
}
