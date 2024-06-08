#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "dimacs/dimacsParser.hpp"
#include "optimizations/blockedClauseElimination/avlIntervalTreeBlockedClauseEliminator.hpp"
#include "optimizations/blockedClauseElimination/candidateSelection/minimumClauseLengthCandidateSelector.hpp"
#include "optimizations/blockedClauseElimination/candidateSelection/overlapBasedCandidateSelector.hpp"
#include "utils/commandLine/commandLineParser.hpp"

enum CandidateSelector
{
	ConsiderAll,
	MinimumClauseLength,
	MinimumNumberOfClauseLiteralOverlaps,
	MaximumNumberOfClauseLiteralOverlaps,
	Unknown
};

inline std::string stringifyCandidateSelectorType(CandidateSelector candidateSelector)
{
	switch (candidateSelector)
	{
	case CandidateSelector::ConsiderAll:
		return "All clauses in formula will be considered as candidate";
	case CandidateSelector::MinimumClauseLength:
		return "Chose candidates based on clause length sorted in ascending order";
	case CandidateSelector::MinimumNumberOfClauseLiteralOverlaps:
		return "Chose candidates based on the number of overlaps with other clauses containing literal of same polarity sorted in ascending order";
	case CandidateSelector::MaximumNumberOfClauseLiteralOverlaps:
		return "Chose candidates based on the number of overlaps with other clauses containing literal of same polarity sorted in descending order";
	default: 
		return "Unknown";
	}
}

int main(int argc, char* argv[])
{
	argc -= (argc > 0);
	argv += (argc > 0); // skip program name argv[0] if present

	std::unique_ptr<commandLineUtils::CommandLineParser> commandLineParser = std::make_unique<commandLineUtils::CommandLineParser>();
	if (!commandLineParser)
	{
		std::cout << "Failed to allocate memory for command line parser\n";
		return EXIT_FAILURE;
	}

	if (!commandLineParser->digest(argc, argv))
	{
		std::cout << "Failed to parse command line arguments\n";
		return EXIT_FAILURE;
	}

	std::unique_ptr<dimacs::DimacsParser> dimacsParser = std::make_unique<dimacs::DimacsParser>();
	if (!dimacsParser)
	{
		std::cout << "Failed to allocate resources for dimacs parser\n";
		return EXIT_FAILURE;
	}

	std::optional<std::string> cnfFormulaFilePath;
	if (!commandLineParser->tryGetStringValue("--cnf", cnfFormulaFilePath) || !cnfFormulaFilePath.has_value())
	{
		std::cout << "Required .cnf file path was not specified\n";
		return EXIT_FAILURE;
	}

	const std::string& dimacsSatFormulaFile = *cnfFormulaFilePath;
	std::vector<std::string> foundErrorsDuringSatFormulaParsingFromFile;

	std::optional<std::size_t> optionalBlockedClauseEliminationMaxNumCandidates;
	if (commandLineParser->tryGetUnsignedNumericValue("--maxNumCandidates", optionalBlockedClauseEliminationMaxNumCandidates, nullptr) && !optionalBlockedClauseEliminationMaxNumCandidates.has_value())
	{
		std::cout << "Failed to parse maximum number of candidates to consider for blocked clause elimination";
		return EXIT_FAILURE;
	}

	CandidateSelector bceCandidateSelectorType = CandidateSelector::ConsiderAll;
	std::optional<std::string> optionalStringifiedCandidateSelectorType;
	if (commandLineParser->tryGetStringValue("--candidateSelector", optionalStringifiedCandidateSelectorType))
	{
		const std::string minClauseLengthBceCandidateSelectorIdentifier = "minClauseLength";
		const std::string minClauseOverlapBceCandidateSelectorIdentifier = "minClauseOverlap";
		const std::string maxClauseOverlapBceCandidateSelectorIdentifier = "maxClauseOverlap";
		const std::string considerAllClausesCandidateSelectorIdentifier = "all";
		if (optionalStringifiedCandidateSelectorType == minClauseLengthBceCandidateSelectorIdentifier)
			bceCandidateSelectorType = CandidateSelector::MinimumClauseLength;
		if (optionalStringifiedCandidateSelectorType == minClauseOverlapBceCandidateSelectorIdentifier)
			bceCandidateSelectorType = CandidateSelector::MinimumNumberOfClauseLiteralOverlaps;
		if (optionalStringifiedCandidateSelectorType == maxClauseOverlapBceCandidateSelectorIdentifier)
			bceCandidateSelectorType = CandidateSelector::MinimumNumberOfClauseLiteralOverlaps;
		if (optionalStringifiedCandidateSelectorType == considerAllClausesCandidateSelectorIdentifier)
			bceCandidateSelectorType = CandidateSelector::ConsiderAll;

		if (bceCandidateSelectorType == CandidateSelector::Unknown)
		{
			std::cout << "Invalid candidate selector type provided";
			return EXIT_FAILURE;
		}
	}

	auto optimizationsConfig = dimacs::DimacsParser::PreprocessingOptimizationsConfig();
	optimizationsConfig.singleLiteralClauseRemovalEnabled = false;
	optimizationsConfig.localClauseLiteralRemovalEnabled = false;

	/*
	 * Prefer the usage of std::chrono::steady_clock instead of std::chrono::sytem_clock since the former cannot decrease (due to time zone changes, etc.) and is most suitable for measuring intervals according to (https://en.cppreference.com/w/cpp/chrono/steady_clock)
	 */
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	std::cout << "=== START - PROCESSING OF DIMACS FORMULA ===\n";
	const TimePoint startTimeForProcessingOfDimacsFormula = std::chrono::steady_clock::now();
	const std::optional<dimacs::ProblemDefinition::ptr>& parsedSatFormula = dimacsParser->readProblemFromFile(dimacsSatFormulaFile, &foundErrorsDuringSatFormulaParsingFromFile, optimizationsConfig);
	if (!parsedSatFormula)
	{
		std::ostringstream out;
		if (!foundErrorsDuringSatFormulaParsingFromFile.empty())
		{
			std::copy(foundErrorsDuringSatFormulaParsingFromFile.cbegin(), std::prev(foundErrorsDuringSatFormulaParsingFromFile.cend()), std::ostream_iterator<std::string>(out, "\r\n"));
			out << foundErrorsDuringSatFormulaParsingFromFile.back();
			std::cerr << out.str() << "\n";
		}
		return EXIT_FAILURE;
	}
	const TimePoint endTimeForProcessingOfDimacsFormula = std::chrono::steady_clock::now();

	std::cout << "Parsing of SAT formula @ " + dimacsSatFormulaFile + " OK\n";
	std::cout << "Parsed formula had " + std::to_string(parsedSatFormula->get()->getClauses()->size()) + " clauses after all preprocessing optimizations were done\n";

	std::vector<long> variablesWithValueDeterminedDuringPreprocessing = parsedSatFormula->get()->getVariablesWithValueDeterminedDuringPreprocessing();
	std::vector<std::string> stringifiedVariablesWithValueDeterminedDuringPreprocessing;
	stringifiedVariablesWithValueDeterminedDuringPreprocessing.reserve(variablesWithValueDeterminedDuringPreprocessing.size());

	std::transform(
		variablesWithValueDeterminedDuringPreprocessing.cbegin(),
		variablesWithValueDeterminedDuringPreprocessing.cend(),
		std::back_inserter(stringifiedVariablesWithValueDeterminedDuringPreprocessing),
		[](const long variable) {return std::to_string(variable); }
	);
	std::ostringstream bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing;

	if (!variablesWithValueDeterminedDuringPreprocessing.empty())
	{
		if (variablesWithValueDeterminedDuringPreprocessing.size() > 1)
		{
			std::copy(stringifiedVariablesWithValueDeterminedDuringPreprocessing.cbegin(), std::prev(stringifiedVariablesWithValueDeterminedDuringPreprocessing.cend()), std::ostream_iterator<std::string>(bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing, " | "));
			bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing << *std::prev(stringifiedVariablesWithValueDeterminedDuringPreprocessing.end());
		}
		else
		{
			bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing << *std::prev(stringifiedVariablesWithValueDeterminedDuringPreprocessing.end());
		}
	}
	else
	{
		bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing << "<NONE>";
	}
	std::cout << "Variables with value determined during preprocessing: " << bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing.str() << "\n";

	const auto durationForProcessingOfDimacsFormula = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForProcessingOfDimacsFormula - startTimeForProcessingOfDimacsFormula).count();
	std::cout << "Duration for processing of DIMACS formula: " + std::to_string(durationForProcessingOfDimacsFormula) + "ms\n";
	std::cout << "=== END - PROCESSING OF DIMACS FORMULA ===\n";

	std::shared_ptr<blockedClauseElimination::BaseCandidateSelector> blockedClauseCandidateSelector;
	switch (bceCandidateSelectorType)
	{
		case CandidateSelector::ConsiderAll:
			blockedClauseCandidateSelector = std::make_shared<blockedClauseElimination::BaseCandidateSelector>(*parsedSatFormula, optionalBlockedClauseEliminationMaxNumCandidates);
			break;
		case CandidateSelector::MinimumClauseLength:
			blockedClauseCandidateSelector = std::make_shared<blockedClauseElimination::MinimumClauseLengthCandidateSelector>(*parsedSatFormula, optionalBlockedClauseEliminationMaxNumCandidates);
			break;
		case CandidateSelector::MinimumNumberOfClauseLiteralOverlaps:
			blockedClauseCandidateSelector = std::make_shared<blockedClauseElimination::OverlapBasedCandidateSelector>(*parsedSatFormula, optionalBlockedClauseEliminationMaxNumCandidates, blockedClauseElimination::OverlapBasedCandidateSelector::MinimumCountsFirst);
			break;
		case CandidateSelector::MaximumNumberOfClauseLiteralOverlaps:
			blockedClauseCandidateSelector = std::make_shared<blockedClauseElimination::OverlapBasedCandidateSelector>(*parsedSatFormula, optionalBlockedClauseEliminationMaxNumCandidates, blockedClauseElimination::OverlapBasedCandidateSelector::MaximumCountsFirst);
			break;
		default:
			std::cerr << "Unhandled blocked clause candidate selector type " << std::to_string(bceCandidateSelectorType) << "\n";
			return EXIT_FAILURE;
	}

	if (!blockedClauseCandidateSelector)
		return EXIT_FAILURE;

	const auto& blockedClauseEliminator = std::make_unique<blockedClauseElimination::AvlIntervalTreeBlockedClauseEliminator>(*parsedSatFormula, blockedClauseCandidateSelector);
	if (!blockedClauseEliminator)
		return EXIT_FAILURE;

	long long benchmarkExecutionTime = 0;

	std::cout << "=== START - BUILDING INTERNAL DATA STRUCTURE FOR BCE CHECK ===\n";
	const TimePoint startTimeForBuildOfInternalDataStructure = std::chrono::steady_clock::now();
	if (!blockedClauseEliminator->initializeInternalHelperStructures())
		return EXIT_FAILURE;

	const TimePoint endTimeForBuildOfInternalDataStructure = std::chrono::steady_clock::now();
	const auto durationForBuildOfInternalDataStructure = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForBuildOfInternalDataStructure - startTimeForBuildOfInternalDataStructure).count();
	std::cout << "Build of internal data structure duration: " + std::to_string(durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "=== END - BUILDING INTERNAL DATA STRUCTURE FOR BCE CHECK ===\n";

	std::cout << "=== START - CANDIDATE GENERATION FOR BCE ===\n";
	std::cout << "Chosen candidate selector: " << stringifyCandidateSelectorType(bceCandidateSelectorType) << "\n";

	const TimePoint startTimeForCandidateGeneration = std::chrono::steady_clock::now();
	const std::vector<std::size_t> indicesOfCandidateClausesForBce = blockedClauseEliminator->determineCandidatesBasedOnHeuristic();
	const TimePoint endTimeForCandidateGeneration = std::chrono::steady_clock::now();

	const auto durationForCandidateGeneration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForCandidateGeneration - startTimeForCandidateGeneration).count();
	std::cout << "Duration for generation of candidates: " + std::to_string(durationForCandidateGeneration) + "ms\n";
	std::cout << "=== END - CANDIDATE GENERATION OR BCE ===\n";

	const std::size_t numClausesToCheck = indicesOfCandidateClausesForBce.size();
	const std::string stringifiedNumClausesToCheck = std::to_string(numClausesToCheck);
	std::cout << stringifiedNumClausesToCheck << " out of all " << std::to_string(parsedSatFormula->get()->getNumClauses()) << " candidate clauses in the formula are chosen as potential candidates\n";

	constexpr std::size_t percentageThreshold = 5;
	const std::size_t numClausesToProcessUntilPercentageThresholdIsReached = numClausesToCheck / (100 / percentageThreshold);
	std::size_t percentThresholdReachedCounter = 0;
	std::size_t remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;

	std::vector<blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult> blockedClauses;
	for (const std::size_t idxOfCandidateClauseInBceCheck : indicesOfCandidateClausesForBce)
	{
		--remainingNumClausesForPercentageThreshold;
		if (!remainingNumClausesForPercentageThreshold)
		{
			remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;
			++percentThresholdReachedCounter;
			std::cout << "Handled [" + std::to_string(percentThresholdReachedCounter * percentageThreshold) + "%] of all clauses, current benchmark duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";
		}

		const TimePoint startTimeForSearchForBlockingLiteralOfClause = std::chrono::steady_clock::now();
		const std::optional<blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult> searchResult = blockedClauseEliminator->isClauseBlocked(idxOfCandidateClauseInBceCheck);

		if (!searchResult.has_value())
		{
			std::cout << "Failed to determine BCE-check result for clause @ index " + std::to_string(idxOfCandidateClauseInBceCheck) + " in formula!";
			return EXIT_FAILURE;
		}
		if (searchResult->isBlocked)
		{
			blockedClauses.emplace_back(searchResult.value());
			if (!blockedClauseEliminator->excludeClauseFromSearchSpace(idxOfCandidateClauseInBceCheck))
			{
				std::cout << "Failed to exclude blocked clause @ index " + std::to_string(idxOfCandidateClauseInBceCheck) + " in formula from further search!";
				return EXIT_FAILURE;
			}
		}

		const TimePoint endTimeForSearchForBlockingLiteralOfClause = std::chrono::steady_clock::now();
		const auto durationForSearchOfBlockingLiteralForClause = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForSearchForBlockingLiteralOfClause - startTimeForSearchForBlockingLiteralOfClause);
		//std::cout << "C: " + std::to_string(i) + " | B: " + std::to_string(searchResult.isBlocked) + "| BY: " + std::to_string(searchResult.idxOfClauseDefiningBlockingLiteral) + "| Duration: " + std::to_string(durationForSearchOfBlockingLiteralForClause.count()) + "ms" << std::endl;
		benchmarkExecutionTime += durationForSearchOfBlockingLiteralForClause.count();
	}

	std::cout << "=== BEGIN - VERIFICATION OF RESULTS ===\n";
	const TimePoint resultVerificationStartTime = std::chrono::steady_clock::now();

	std::cout << "SKIPPED FOR NOW...\n";

	const TimePoint resultVerificationEndTime = std::chrono::steady_clock::now();
	const auto resultVerificationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(resultVerificationEndTime - resultVerificationStartTime).count();
	std::cout << "=== BEGIN - VERIFICATION OF RESULTS ===\n";

	std::cout << "=== RESULTS ===\n";
	std::cout << "Are blocked clauses removed from search space: " + std::to_string(true) + "\n";
	std::cout << "Candidate selector: " << stringifyCandidateSelectorType(bceCandidateSelectorType) << "\n";
	std::cout << "Number of candidates considered: " << std::to_string(indicesOfCandidateClausesForBce.size()) << " out of " << stringifiedNumClausesToCheck << "\n";
	std::cout << std::to_string(blockedClauses.size()) + " clauses out of " + std::to_string(numClausesToCheck) + " were blocked!\n\n";

	std::cout << "Duration for processing of DIMACS formula: " + std::to_string(durationForProcessingOfDimacsFormula) + "ms\n";
	std::cout << "Duration for generation of candidate: " + std::to_string(durationForCandidateGeneration) + "ms\n";
	std::cout << "Build of internal data structure duration: " + std::to_string(durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "BCE check duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";
	std::cout << "BCE result verification duration: " + std::to_string(resultVerificationDuration) + "ms\n";

	const long long totalDuration = durationForProcessingOfDimacsFormula + durationForCandidateGeneration + durationForBuildOfInternalDataStructure + benchmarkExecutionTime;
	std::cout << "TOTAL: " + std::to_string(totalDuration) + "ms\n";
	std::cout << "TOTAL (excluding processing of DIMACS formula): " + std::to_string(totalDuration - durationForProcessingOfDimacsFormula) + "ms\n";
	return EXIT_SUCCESS;
}
