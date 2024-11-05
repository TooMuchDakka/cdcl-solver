#include <gtest/gtest.h>

#include <dimacs/dimacsParser.hpp>

using namespace dimacs;

class DimacsParserTests : public testing::Test {
public:
	static void parserCnfFormulaWithErrors(const std::string& stringifiedCnfFormulaDefinition, const std::vector<dimacs::DimacsParser::ProcessingError>& expectedErrors)
	{
		auto parserInstance = std::make_unique<DimacsParser>();
		ASSERT_TRUE(parserInstance);

		std::optional<dimacs::ProblemDefinition::ptr> temporaryCnfFormulaStorage;
		std::vector<dimacs::DimacsParser::ProcessingError> actualErrors;
		ASSERT_NO_THROW(temporaryCnfFormulaStorage = parserInstance->readProblemFromString(stringifiedCnfFormulaDefinition, &actualErrors));
		ASSERT_TRUE(temporaryCnfFormulaStorage.has_value());
	}

	static void parseCnfFormulaWithoutErrors(const std::string& stringifiedCnfFormulaDefinition, dimacs::ProblemDefinition::ptr& parsedCnfFormula)
	{
		auto parserInstance = std::make_unique<DimacsParser>();
		ASSERT_TRUE(parserInstance);

		std::optional<dimacs::ProblemDefinition::ptr> temporaryCnfFormulaStorage;
		ASSERT_NO_THROW(temporaryCnfFormulaStorage = parserInstance->readProblemFromString(stringifiedCnfFormulaDefinition, nullptr));
		ASSERT_TRUE(temporaryCnfFormulaStorage.has_value());

		parsedCnfFormula = *temporaryCnfFormulaStorage;
	}
};

TEST_F(DimacsParserTests, ProcessFormulaWithoutUnitPropagation) {
	dimacs::ProblemDefinition::ptr cnfFormula;
	ASSERT_NO_FATAL_FAILURE(parseCnfFormulaWithoutErrors(
		"p cnf 3 4\n1 2 3 0\n 1 2 0\n 2 0\n 3 1 0\n 2 3 0", 
		cnfFormula));
}