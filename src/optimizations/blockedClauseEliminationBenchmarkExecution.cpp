#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "dimacs/dimacsParser.hpp"
#include "optimizations/avlIntervalTreeBlockedClauseEliminator.hpp"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Solver expected only one argument which defines the path to the SAT formula\n";
		return EXIT_FAILURE;
	}

	std::unique_ptr<dimacs::DimacsParser> dimacsParser = std::make_unique<dimacs::DimacsParser>();
	if (!dimacsParser)
	{
		std::cerr << "Failed to allocate resources for dimacs parser\n";
		return EXIT_FAILURE;
	}

	const std::string dimacsSatFormulaFile = argv[1];
	std::vector<std::string> foundErrorsDuringSatFormulaParsingFromFile;

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

	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::AvlIntervalTreeBlockedClauseEliminator>(*parsedSatFormula);
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

	const std::size_t numClausesToCheck = parsedSatFormula->get()->getClauses()->size();
	const std::string stringifiedNumClausesToCheck = std::to_string(numClausesToCheck);

	constexpr std::size_t percentageThreshold = 5;
	const std::size_t numClausesToProcessUntilPercentageThresholdIsReached = numClausesToCheck / (100 / percentageThreshold);
	std::size_t percentThresholdReachedCounter = 0;
	std::size_t remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;

	std::vector<blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult> blockedClauses;
	for (std::size_t i = 0; i < numClausesToCheck; ++i)
	{
		--remainingNumClausesForPercentageThreshold;
		if (!remainingNumClausesForPercentageThreshold)
		{
			remainingNumClausesForPercentageThreshold = numClausesToProcessUntilPercentageThresholdIsReached;
			++percentThresholdReachedCounter;
			std::cout << "Handled [" + std::to_string(percentThresholdReachedCounter * percentageThreshold) + "%] of all clauses, current benchmark duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";
		}

		const TimePoint startTimeForSearchForBlockingLiteralOfClause = std::chrono::steady_clock::now();
		const std::optional<blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult> searchResult = blockedClauseEliminator->isClauseBlocked(i);
		const TimePoint endTimeForSearchForBlockingLiteralOfClause = std::chrono::steady_clock::now();

		if (!searchResult.has_value())
		{
			std::cout << "Failed to determine BCE-check result for clause @ index " + std::to_string(i) + " in formula!";
			return EXIT_FAILURE;
		}
		if (searchResult->isBlocked)
		{
			blockedClauses.emplace_back(searchResult.value());
			if (!blockedClauseEliminator->excludeClauseFromSearchSpace(i))
			{
				std::cout << "Failed to exclude blocked clause @ index " + std::to_string(i) + " in formula from further search!";
				return EXIT_FAILURE;
			}
		}

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
	std::cout << std::to_string(blockedClauses.size()) + " clauses out of " + std::to_string(numClausesToCheck) + " were blocked!\n";
	std::cout << "Duration for processing of DIMACS formula: " + std::to_string(durationForProcessingOfDimacsFormula) + "ms\n";
	std::cout << "Build of internal data structure duration: " + std::to_string(durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "BCE check duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";
	std::cout << "BCE result verification duration: " + std::to_string(resultVerificationDuration) + "ms\n";
	std::cout << "TOTAL: " + std::to_string( durationForProcessingOfDimacsFormula + benchmarkExecutionTime + durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "TOTAL (excluding processing of DIMACS formula): " + std::to_string(benchmarkExecutionTime - durationForProcessingOfDimacsFormula) + "ms\n";
	return EXIT_SUCCESS;
}