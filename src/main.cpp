#include <cstdlib>
#include <iostream>
#include <sstream>

#include "dimacs/dimacsParser.hpp"

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
	std::vector<dimacs::DimacsParser::ProcessingError> foundErrorsDuringSatFormulaParsingFromFile;

	const std::optional<dimacs::ProblemDefinition::ptr>& parsedSatFormula = dimacsParser->readProblemFromFile(dimacsSatFormulaFile, &foundErrorsDuringSatFormulaParsingFromFile);
	if (!parsedSatFormula)
	{
		if (!foundErrorsDuringSatFormulaParsingFromFile.empty())
		{
			std::ostringstream out;
			for (const auto& processingError : foundErrorsDuringSatFormulaParsingFromFile)
				out << processingError << "\r\n";

			if (foundErrorsDuringSatFormulaParsingFromFile.size() > 1)
				out << foundErrorsDuringSatFormulaParsingFromFile.back();

			std::cerr << out.str() << "\n";
		}
		return EXIT_FAILURE;
	}
	std::cout << "Parsing of SAT formula @ " + dimacsSatFormulaFile + " OK\n";
	return EXIT_SUCCESS;
}
