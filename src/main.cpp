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
	const dimacs::DimacsParser::ParseResult parsingResult = dimacsParser->readProblemFromFile(dimacsSatFormulaFile);
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
	std::cout << "Parsing of SAT formula @ " + dimacsSatFormulaFile + " OK\n";
	return EXIT_SUCCESS;
}
