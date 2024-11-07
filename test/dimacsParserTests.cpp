#include <gtest/gtest.h>

#include <dimacs/dimacsParser.hpp>
#include "problemDefinitionBuilder.hpp"

using namespace dimacs;

class DimacsParserTests : public testing::Test {
public:
	struct ClauseAndFormulaIndexPair
	{
		std::size_t indexInFormula;
		ProblemDefinition::Clause clause;
	};

	struct ExpectedVariableValueLookup
	{
		std::unordered_map<std::size_t, ProblemDefinition::VariableValue> valueLookup;
		ExpectedVariableValueLookup(std::size_t numVariables, const std::initializer_list<std::pair<std::size_t, ProblemDefinition::VariableValue>>& expectedVariableValues)
			: ExpectedVariableValueLookup(numVariables)
		{
			for (const auto& [variable, expectedValue] : expectedVariableValues)
				valueLookup[variable] = expectedValue;
		}

		ExpectedVariableValueLookup(std::size_t numVariables)
		{
			for (std::size_t i = 1; i < numVariables; ++i)
				valueLookup.emplace(i, ProblemDefinition::VariableValue::Unknown);
		}
	};

	struct ExpectedOverlappedClausesPerLiteralLookup
	{
		std::unordered_map<long, std::vector<std::size_t>> lookup;
		ExpectedOverlappedClausesPerLiteralLookup(std::size_t numVariables, const std::initializer_list<std::pair<long, std::vector<std::size_t>>>& expectedOverlappedClausePerLiteralLookup)
		{
			for (std::size_t i = 1; i < numVariables; ++i)
			{
				lookup.emplace(std::make_pair<long, std::vector<std::size_t>>(i, {}));
				lookup.emplace(std::make_pair<long, std::vector<std::size_t>>(-i, {}));
			}

			for (const auto& [literal, overlappedClauseIndices] : expectedOverlappedClausePerLiteralLookup)
			{
				std::vector<std::size_t>& lookupEntry = lookup[literal];
				lookupEntry.reserve(overlappedClauseIndices.size());
				for (const std::size_t overlappedClauseIndex : overlappedClauseIndices)
					lookupEntry.emplace_back(overlappedClauseIndex);
			}
		}
	};

	struct ExpectedOverlappedClauseCountPerLiteralLookup
	{
		std::unordered_map<long, std::size_t> lookup;
		ExpectedOverlappedClauseCountPerLiteralLookup(std::size_t numVariables, const std::initializer_list<std::pair<long, std::size_t>>& overlappedClauseCountPerLiteral)
		{
			for (std::size_t i = 1; i < numVariables; ++i)
			{
				lookup.emplace(-i, 0);
				lookup.emplace(i, 0);
			}

			for (const auto& [literal, count] : overlappedClauseCountPerLiteral)
				lookup[literal] +=count;
		}
	};

	static void parserCnfFormulaWithErrors(const std::string& stringifiedCnfFormulaDefinition, const std::vector<dimacs::DimacsParser::ProcessingError>& expectedErrors, bool compareErrorTexts)
	{
		auto parserInstance = std::make_unique<DimacsParser>();
		ASSERT_TRUE(parserInstance);

		std::optional<dimacs::ProblemDefinition::ptr> temporaryCnfFormulaStorage;
		std::vector<dimacs::DimacsParser::ProcessingError> actualErrors;
		ASSERT_NO_THROW(temporaryCnfFormulaStorage = parserInstance->readProblemFromString(stringifiedCnfFormulaDefinition, &actualErrors));
		ASSERT_FALSE(temporaryCnfFormulaStorage.has_value());

		ASSERT_EQ(expectedErrors.size(), actualErrors.size());
		for (std::size_t i = 0; i < expectedErrors.size(); ++i)
		{
			const DimacsParser::ProcessingError& expectedError = expectedErrors.at(i);
			const DimacsParser::ProcessingError& actualError = actualErrors.at(i);
			ASSERT_EQ(expectedError.position.has_value(), actualError.position.has_value());
			if (expectedError.position.has_value())
			{
				ASSERT_EQ(expectedError.position->line, actualError.position->line);
				ASSERT_EQ(expectedError.position->column, actualError.position->column);
			}

			if (compareErrorTexts)
				ASSERT_EQ(expectedError.text, actualError.text);
		}
	}

	static void parseCnfFormulaWithoutErrors(const std::string& stringifiedCnfFormulaDefinition, const DimacsParser::ParserConfiguration& parserConfiguration, dimacs::ProblemDefinition::ptr& parsedCnfFormula)
	{
		auto parserInstance = std::make_unique<DimacsParser>(parserConfiguration);
		ASSERT_TRUE(parserInstance);

		std::optional<dimacs::ProblemDefinition::ptr> temporaryCnfFormulaStorage;
		std::vector<DimacsParser::ProcessingError> foundProcessingErrors;
		ASSERT_NO_THROW(temporaryCnfFormulaStorage = parserInstance->readProblemFromString(stringifiedCnfFormulaDefinition, &foundProcessingErrors));
		ASSERT_TRUE(temporaryCnfFormulaStorage.has_value());

		parsedCnfFormula = *temporaryCnfFormulaStorage;
	}

	static void assertCnfHeaderEquality(std::size_t expectedNumVariables, std::size_t expectedNumClauses, const ProblemDefinition& formula)
	{
		ASSERT_EQ(expectedNumVariables, formula.getNumDeclaredVariablesOfFormula());
		ASSERT_EQ(expectedNumClauses, formula.getNumDeclaredClausesOfFormula());
	}

	static void assertParsedClausesEquality(const std::vector<ClauseAndFormulaIndexPair>& expectedClauses, const ProblemDefinition& formula)
	{
		ASSERT_EQ(expectedClauses.size(), formula.getNumClausesAfterOptimizations());
		for (const ClauseAndFormulaIndexPair clauseAndFormulaIndexPair : expectedClauses)
		{
			const std::optional<const ProblemDefinition::Clause*> actualClause = formula.getClauseByIndexInFormula(clauseAndFormulaIndexPair.indexInFormula);
			ASSERT_TRUE(actualClause.has_value());
			ASSERT_NO_FATAL_FAILURE(assertClausesMatch(clauseAndFormulaIndexPair.clause, **actualClause));
		}
	}

	static void assertVariableValueEquality(const ExpectedVariableValueLookup& expectedVariableValueLookup, const ProblemDefinition& formula)
	{
		for (const auto& [variable, expectedValue] : expectedVariableValueLookup.valueLookup)
		{
			const std::optional<ProblemDefinition::VariableValue> actualVariableValue = formula.getValueOfVariable(variable);
			ASSERT_TRUE(actualVariableValue.has_value());
			ASSERT_EQ(expectedValue, actualVariableValue.value());
		}
	}

	static void assertNumOverlappedClausePerLiteralEquality(const ExpectedOverlappedClauseCountPerLiteralLookup& expectedCountsPerLiteral, const ProblemDefinition& formula)
	{
		const LiteralOccurrenceLookup& actualLiteralOccurrenceLookup = formula.getLiteralOccurrenceLookup();
		for (const auto& [literal, expectedNumOverlaps] : expectedCountsPerLiteral.lookup)
		{
			const std::optional<std::size_t>& actualNumOccurrencesOfLiteral = actualLiteralOccurrenceLookup.getNumberOfOccurrencesOfLiteral(literal);
			ASSERT_TRUE(actualNumOccurrencesOfLiteral.has_value());
			ASSERT_EQ(expectedNumOverlaps, *actualNumOccurrencesOfLiteral);
		}
	}

	static void assertOverlappedClausesPerLiteralEquality(const ExpectedOverlappedClausesPerLiteralLookup& expectedOverlapsPerLiteral, const ProblemDefinition& formula)
	{
		const LiteralOccurrenceLookup& actualLiteralOccurrenceLookup = formula.getLiteralOccurrenceLookup();
		for (const auto& [literal, expectedOverlaps] : expectedOverlapsPerLiteral.lookup)
		{
			std::unordered_set<std::size_t> unordedLookupForExpectedOverlaps(expectedOverlaps.cbegin(), expectedOverlaps.cend());
			const std::optional<std::vector<std::size_t>>& actualOverlappedClauses = actualLiteralOccurrenceLookup[literal];
			ASSERT_TRUE(actualOverlappedClauses.has_value());
			ASSERT_EQ(unordedLookupForExpectedOverlaps.size(), actualOverlappedClauses->size());

			for (const std::size_t actualOverlappedClauseIndex : *actualOverlappedClauses)
				ASSERT_TRUE(unordedLookupForExpectedOverlaps.count(actualOverlappedClauseIndex));
		}
	}

	static void assertClausesMatch(const dimacs::ProblemDefinition::Clause& expected, const dimacs::ProblemDefinition::Clause& actual)
	{
		ASSERT_EQ(expected.literals.size(), actual.literals.size());
		for (std::size_t i = 0; i < expected.literals.size(); ++i)
			ASSERT_EQ(expected.literals.at(i), actual.literals.at(i));
	}
};

// SUCCESS CASES
TEST_F(DimacsParserTests, FormulaWithClauseLiteralsOfDifferentPolarityAndNoUnitPropagationPerformed) {
	constexpr auto parserConfiguration = DimacsParser::ParserConfiguration({ false });
	dimacs::ProblemDefinition::ptr cnfFormula;

	ASSERT_NO_FATAL_FAILURE(parseCnfFormulaWithoutErrors(
		"p cnf 3 5\n1 -2 3 0\n 1 2 0\n 2 0\n -3 -1 0\n 2 3 0", 
		parserConfiguration, cnfFormula));
	ASSERT_TRUE(cnfFormula);

	constexpr std::size_t expectedNumVariables = 3;
	constexpr std::size_t expectedNumClauses = 5;

	const std::vector<ClauseAndFormulaIndexPair> expectedClauses = {
		ClauseAndFormulaIndexPair({0, ProblemDefinition::Clause({1,-2,3})}),
		ClauseAndFormulaIndexPair({1, ProblemDefinition::Clause({1,2})}),
		ClauseAndFormulaIndexPair({2,ProblemDefinition::Clause({2})}),
		ClauseAndFormulaIndexPair({3,ProblemDefinition::Clause({-3,-1})}),
		ClauseAndFormulaIndexPair({4,ProblemDefinition::Clause({2,3})})
	};

	const ExpectedVariableValueLookup expectedVariableValueLookup(expectedNumVariables);
	const ExpectedOverlappedClauseCountPerLiteralLookup expectedOverlappedClauseCountPerLiteralLookup(expectedNumVariables, {
		{-3, 1}, {-2, 1}, { -1, 1 }, {1, 2}, {2, 3}, {3, 2}
	});
	const ExpectedOverlappedClausesPerLiteralLookup expectedOverlappedClausesPerLiteralLookup(expectedNumVariables, {
		{-3, {3}}, {-2, {0}}, {-1, {3}}, {1, {0, 1}}, {2, {1, 2, 4}}, {3, {0,4}}
	});

	ASSERT_NO_FATAL_FAILURE(assertCnfHeaderEquality(expectedNumVariables, expectedNumClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertParsedClausesEquality(expectedClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertOverlappedClausesPerLiteralEquality(expectedOverlappedClausesPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertNumOverlappedClausePerLiteralEquality(expectedOverlappedClauseCountPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertVariableValueEquality(expectedVariableValueLookup, *cnfFormula));
}

TEST_F(DimacsParserTests, UnitPropagationInFormulaPerformed)
{
	constexpr auto parserConfiguration = DimacsParser::ParserConfiguration({ true });
	dimacs::ProblemDefinition::ptr cnfFormula;

	ASSERT_NO_FATAL_FAILURE(parseCnfFormulaWithoutErrors(
		"p cnf 3 5\n1 -2 3 0\n 1 2 0\n 2 0\n -3 -1 0\n 2 3 0",
		parserConfiguration, cnfFormula));
	ASSERT_TRUE(cnfFormula);

	constexpr std::size_t expectedNumVariables = 3;
	constexpr std::size_t expectedNumClauses = 5;

	const std::vector<ClauseAndFormulaIndexPair> expectedClauses = {
		ClauseAndFormulaIndexPair({0, ProblemDefinition::Clause({1,3})}),
		ClauseAndFormulaIndexPair({3,ProblemDefinition::Clause({-3,-1})})
	};

	const ExpectedVariableValueLookup expectedVariableValueLookup(expectedNumVariables, {{2, ProblemDefinition::VariableValue::High}});
	const ExpectedOverlappedClauseCountPerLiteralLookup expectedOverlappedClauseCountPerLiteralLookup(expectedNumVariables, {
		{-3, 1}, {-1, 1}, {1, 1}, {3, 1}
	});
	const ExpectedOverlappedClausesPerLiteralLookup expectedOverlappedClausesPerLiteralLookup(expectedNumVariables, {
		{-3, {3}}, {-1, {3}}, {1, {0}}, {3, {0}}
	});

	ASSERT_NO_FATAL_FAILURE(assertCnfHeaderEquality(expectedNumVariables, expectedNumClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertParsedClausesEquality(expectedClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertOverlappedClausesPerLiteralEquality(expectedOverlappedClausesPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertNumOverlappedClausePerLiteralEquality(expectedOverlappedClauseCountPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertVariableValueEquality(expectedVariableValueLookup, *cnfFormula));
}

TEST_F(DimacsParserTests, UnitPropagationCausesSubsequentPropagations)
{
	constexpr auto parserConfiguration = DimacsParser::ParserConfiguration({ true });
	dimacs::ProblemDefinition::ptr cnfFormula;

	ASSERT_NO_FATAL_FAILURE(parseCnfFormulaWithoutErrors(
		"p cnf 3 5\n-2 3 0\n 1 2 0\n 2 0\n -3 -1 0\n 2 3 0",
		parserConfiguration, cnfFormula));
	ASSERT_TRUE(cnfFormula);

	constexpr std::size_t expectedNumVariables = 3;
	constexpr std::size_t expectedNumClauses = 5;

	const std::vector<ClauseAndFormulaIndexPair> expectedClauses;
	const ExpectedVariableValueLookup expectedVariableValueLookup(expectedNumVariables, { {2, ProblemDefinition::VariableValue::High}, {3, ProblemDefinition::VariableValue::High}, {1, ProblemDefinition::VariableValue::Low} });
	const ExpectedOverlappedClauseCountPerLiteralLookup expectedOverlappedClauseCountPerLiteralLookup(expectedNumVariables, {});
	const ExpectedOverlappedClausesPerLiteralLookup expectedOverlappedClausesPerLiteralLookup(expectedNumVariables, {});

	ASSERT_NO_FATAL_FAILURE(assertCnfHeaderEquality(expectedNumVariables, expectedNumClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertParsedClausesEquality(expectedClauses, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertOverlappedClausesPerLiteralEquality(expectedOverlappedClausesPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertNumOverlappedClausePerLiteralEquality(expectedOverlappedClauseCountPerLiteralLookup, *cnfFormula));
	ASSERT_NO_FATAL_FAILURE(assertVariableValueEquality(expectedVariableValueLookup, *cnfFormula));
}

TEST_F(DimacsParserTests, LocalVariablePropagatedInFormula)
{
	GTEST_SKIP();
}

TEST_F(DimacsParserTests, ConflictDuringUnitPropagationDetectedCorrectly)
{
	GTEST_SKIP();
}

// ERROR CASES
TEST_F(DimacsParserTests, InvalidHeaderPrefixDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1,0,"") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"t cnf 2 1\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, MissingCnfInfixInHeaderDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p test 2 1\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NonNumericNumberOfVariablesInHeaderDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf test 1\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NonNumericNumberOfClausesInHeaderDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 test\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NegativeNumberOfVariablesInHeaderDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf -2 1\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NegativeNumberOfClausesInHeaderDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(1, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 -1\n1 2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NotDeclaredVariableDetectedInClause)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(2, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 1\n1 3 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, MoreUserDefinedClausesThanDeclaredOnesDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(3, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 1\n1 2 0\n-2 1 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, LessUserDefinedClausesThanDeclaredOnesDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(3, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 3\n1 2 0\n-2 1 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, MissingClausePostfixDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(2, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 1\n1 2",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, InvalidUsageOfClausePostfixAsLiteralDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(2, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 1\n1 0 2",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, TautologyInFormulaNotSupported)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(3, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 3\n1 2 0\n 2 -2 0\n -1 -2 0",
		expectedErrors, false));
}

TEST_F(DimacsParserTests, NonNumericClauseLiteralDetected)
{
	const std::vector<DimacsParser::ProcessingError>& expectedErrors = { DimacsParser::ProcessingError(3, 0, "") };
	ASSERT_NO_FATAL_FAILURE(parserCnfFormulaWithErrors(
		"p cnf 2 2\n1 2 0\n -1 test 0",
		expectedErrors, false));
}