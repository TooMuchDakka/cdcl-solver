#include <cstdlib>
#include <iostream>
#include <sstream>

#include "dimacs/dimacsParser.hpp"

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
	return EXIT_SUCCESS;
}