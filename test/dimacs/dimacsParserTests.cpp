#include <gtest/gtest.h>
#include "dimacs/dimacsParser.hpp"
#include "dimacsParserTestFixture.hpp"

TEST_F(DimacsParserTestFixture, BasicTest)
{
	const dimacs::DimacsParser::ptr parserInstance = std::make_unique<dimacs::DimacsParser>();
	EXPECT_TRUE(parserInstance);

	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}