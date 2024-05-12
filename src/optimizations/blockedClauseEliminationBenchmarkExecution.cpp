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
		std::cerr << "Solver expected only one argument which defines the path to the SAT formula" << std::endl;
		return EXIT_FAILURE;
	}

	std::unique_ptr<dimacs::DimacsParser> dimacsParser = std::make_unique<dimacs::DimacsParser>();
	if (!dimacsParser)
	{
		std::cerr << "Failed to allocate resources for dimacs parser" << std::endl;
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
			std::copy(foundErrorsDuringSatFormulaParsingFromFile.cbegin(), std::prev(foundErrorsDuringSatFormulaParsingFromFile.cend(), 2), std::ostream_iterator<std::string>(out, "\r\n"));
			out << foundErrorsDuringSatFormulaParsingFromFile.back();
			std::cerr << out.str() << std::endl;
		}
		return EXIT_FAILURE;
	}
	std::cout << "Parsing of SAT formula @ " + dimacsSatFormulaFile + " OK" << std::endl;
	std::cout << "Parsed formula had " + std::to_string(parsedSatFormula->get()->getClauses()->size()) + " clauses after all preprocessing optimizations were done" << std::endl;

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
			std::copy(stringifiedVariablesWithValueDeterminedDuringPreprocessing.cbegin(), std::prev(stringifiedVariablesWithValueDeterminedDuringPreprocessing.cend(), 2), std::ostream_iterator<std::string>(bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing, " | "));
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
	std::cout << "Variables with value determined during preprocessing: " << bufferForStringifiedVariableIdentsWithValueDeterminedDuringPreprocessing.str() << std::endl;


	const std::unique_ptr<blockedClauseElimination::BaseBlockedClauseEliminator> blockedClauseEliminator = std::make_unique<blockedClauseElimination::BaseBlockedClauseEliminator>(*parsedSatFormula);
	if (!blockedClauseEliminator)
		return EXIT_FAILURE;

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
	long long benchmarkExecutionTime = 0;
	
	const std::size_t numClausesToCheck = parsedSatFormula->get()->getClauses()->size();
	for (std::size_t i = 0; i < numClausesToCheck; ++i)
	{
		const TimePoint startTimeForSearchForBlockingLiteralOfClause = std::chrono::system_clock::now();
		const blockedClauseElimination::BaseBlockedClauseEliminator::BlockedClauseSearchResult searchResult = *blockedClauseEliminator->isClauseBlocked(i);
		const TimePoint endTimeForSearchForBlockingLiteralOfClause = std::chrono::system_clock::now();

		const auto durationForSearchOfBlockingLiteralForClause = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeForSearchForBlockingLiteralOfClause - startTimeForSearchForBlockingLiteralOfClause);
		std::cout << "Clause " + std::to_string(i) + " blocked: " + std::to_string(searchResult.isBlocked) + " by clause @ index " + std::to_string(searchResult.idxOfClauseDefiningBlockingLiteral) + " in formula | Check duration: " + std::to_string(durationForSearchOfBlockingLiteralForClause.count()) + "ms" << std::endl;
		benchmarkExecutionTime += durationForSearchOfBlockingLiteralForClause.count();
	}
	std::cout << "Total duration: " + std::to_string(benchmarkExecutionTime) + "ms" << std::endl;
	return EXIT_SUCCESS;
}