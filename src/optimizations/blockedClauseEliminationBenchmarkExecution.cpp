#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "dimacs/dimacsParser.hpp"
#include "optimizations/blockedClauseElimination.hpp"

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
	optimizationsConfig.singleLiteralClauseRemovalEnabled = true;
	optimizationsConfig.localClauseLiteralRemovalEnabled = true;

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


	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(*parsedSatFormula);
	if (!blockedClauseEliminator)
		return EXIT_FAILURE;

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
	long long benchmarkExecutionTime = 0;

	std::cout << "=== START - BUILDING INTERNAL DATA STRUCTURE FOR BCE CHECK ===\n";
	const TimePoint startTimeForBuildOfInternalDataStructure = std::chrono::system_clock::now();
	if (!blockedClauseEliminator->initializeInternalHelperStructures())
		return EXIT_FAILURE;

	const TimePoint endTimeForBuildOfInternalDataStructure = std::chrono::system_clock::now();
	const auto durationForBuildOfInternalDataStructure = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForBuildOfInternalDataStructure - startTimeForBuildOfInternalDataStructure).count();
	std::cout << "Build of internal data structure duration: " + std::to_string(durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "=== END - BUILDING INTERNAL DATA STRUCTURE FOR BCE CHECK ===\n";

	const std::size_t numClausesToCheck = parsedSatFormula->get()->getClauses()->size();
	const std::string stringifiedNumClausesToCheck = std::to_string(numClausesToCheck);
	for (std::size_t i = 0; i < numClausesToCheck; ++i)
	{
		if (i % 2500 == 0 || i == numClausesToCheck - 1)
			std::cout << "Handled [" + std::to_string(i + 1) + "|" + stringifiedNumClausesToCheck + "] clauses, current benchmark duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";

		const TimePoint startTimeForSearchForBlockingLiteralOfClause = std::chrono::system_clock::now();
		const std::optional<blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult> searchResult = blockedClauseEliminator->isClauseBlocked(i);
		const TimePoint endTimeForSearchForBlockingLiteralOfClause = std::chrono::system_clock::now();

		if (!searchResult.has_value())
		{
			std::cout << "Failed to determine BCE-check result for clause @ index " + std::to_string(i) + " in formula!";
			return EXIT_FAILURE;
		}
		const auto durationForSearchOfBlockingLiteralForClause = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForSearchForBlockingLiteralOfClause - startTimeForSearchForBlockingLiteralOfClause);
		//std::cout << "C: " + std::to_string(i) + " | B: " + std::to_string(searchResult.isBlocked) + "| BY: " + std::to_string(searchResult.idxOfClauseDefiningBlockingLiteral) + "| Duration: " + std::to_string(durationForSearchOfBlockingLiteralForClause.count()) + "ms" << std::endl;
		benchmarkExecutionTime += durationForSearchOfBlockingLiteralForClause.count();
	}
	std::cout << "=== RESULTS ===\n";
	std::cout << "BCE check duration: " + std::to_string(benchmarkExecutionTime) + "ms\n";
	std::cout << "Build of internal data structure duration: " + std::to_string(durationForBuildOfInternalDataStructure) + "ms\n";
	std::cout << "TOTAL: " + std::to_string(benchmarkExecutionTime + durationForBuildOfInternalDataStructure) + "ms\n";
	return EXIT_SUCCESS;
}