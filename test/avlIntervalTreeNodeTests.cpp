#include <gtest/gtest.h>
#include <optimizations/utils/avlIntervalTreeNode.hpp>

#include <unordered_set>

#include "dimacs/problemDefinition.hpp"
using namespace avl;

class AvlIntervalTreeNodeTests : public testing::Test {
public:
	struct ClauseIndexAndLiterals
	{
		std::size_t index;
		long literalsLowerBound;
		long literalUpperBounds;

		explicit ClauseIndexAndLiterals(std::size_t index, const std::pair<long, long>& literalBounds)
			: index(index), literalsLowerBound(literalBounds.first), literalUpperBounds(literalBounds.second) {}
	};

	static void assertClauseBoundsAndIndicesMatch(const AvlIntervalTreeNode::ClauseBoundsAndIndices& expected, const AvlIntervalTreeNode::ClauseBoundsAndIndices& actual)
	{
		ASSERT_EQ(expected.literalBoundsSortOrder, actual.literalBoundsSortOrder);
		ASSERT_NO_FATAL_FAILURE(assertContainersMatchInAnyOrder<long>(expected.literalBounds, actual.literalBounds));
		ASSERT_NO_FATAL_FAILURE(assertContainersMatchInAnyOrder<std::size_t>(expected.clauseIndices, actual.clauseIndices));
	}

	template<typename Elem>
	static void assertContainersMatchInAnyOrder(const std::vector<Elem>& expected, const std::vector<Elem>& actual)
	{
		ASSERT_EQ(expected.size(), actual.size());
		std::unordered_set<Elem> expectedLookupSet(expected.cbegin(), expected.cbegin());
		for (const Elem actualElement : expectedLookupSet)
			ASSERT_TRUE(expectedLookupSet.count(actualElement));
	}

	static void assertContainersMatchExactly(const std::vector<std::size_t>& expected, const std::vector<std::size_t>& actual)
	{
		ASSERT_EQ(expected.size(), actual.size());
		for (std::size_t i = 0; i < expected.size(); ++i)
			ASSERT_EQ(expected.at(i), actual.at(i));
	}

	static void insertClauseIndexAndLiteralsOrThrow(AvlIntervalTreeNode::ClauseBoundsAndIndices& container, std::size_t index, long literalBound)
	{
		ASSERT_TRUE(container.insertClause(index, literalBound));
	}

	static void inserClauseIndexAndLiteralsOrThrow(AvlIntervalTreeNode& avlTreeNode, const ClauseIndexAndLiterals& clauseIndexAndLiterals)
	{
		ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlTreeNode.overlappingIntervalsLowerBoundsData, clauseIndexAndLiterals.index, clauseIndexAndLiterals.literalsLowerBound));
		ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlTreeNode.overlappingIntervalsUpperBoundsData, clauseIndexAndLiterals.index, clauseIndexAndLiterals.literalUpperBounds));
	}

	static void insertClauseIndicesAndLiteralsOrThrow(AvlIntervalTreeNode& avlTreeNode, const std::vector<ClauseIndexAndLiterals>& clauseIndicesAndLiterals)
	{
		for (const ClauseIndexAndLiterals& clauseIndexAndLiterals : clauseIndicesAndLiterals)
			ASSERT_NO_FATAL_FAILURE(inserClauseIndexAndLiteralsOrThrow(avlTreeNode, clauseIndexAndLiterals));
	}

	static void assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(const AvlIntervalTreeNode::ClauseBoundsAndIndices& container, long literal, const std::vector<std::size_t>& expectedIndicesOfClausesPotentiallyOverlappingLiteral)
	{
		std::vector<std::size_t> actualIndicesOfClausesPotentiallyOverlappingLiteral;
		ASSERT_NO_FATAL_FAILURE(actualIndicesOfClausesPotentiallyOverlappingLiteral = container.getIndicesOfClausesOverlappingLiteralBound(literal));
		assertContainersMatchInAnyOrder<std::size_t>(expectedIndicesOfClausesPotentiallyOverlappingLiteral, actualIndicesOfClausesPotentiallyOverlappingLiteral);
	}
};

TEST_F(AvlIntervalTreeNodeTests, CheckConstructionOfNode) {
	constexpr long expectedIntervalMidPoint = 3;
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(expectedIntervalMidPoint, nullptr);
	ASSERT_EQ(avlIntervalTreeNode->balancingFactor, AvlIntervalTreeNode::BalancingFactor::Balanced);
	ASSERT_FALSE(avlIntervalTreeNode->parent);
	ASSERT_FALSE(avlIntervalTreeNode->left);
	ASSERT_FALSE(avlIntervalTreeNode->right);

	ASSERT_EQ(avlIntervalTreeNode->intervalMidPoint, expectedIntervalMidPoint);

	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));

	auto expectedLowerClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending);
	expectedLowerClauseBoundsAndIndices.clauseIndices = { 0,1,2,3,4 };
	expectedLowerClauseBoundsAndIndices.literalBounds = { -4,-3,-3,-2,-1 };

	auto expectedUpperClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending);
	expectedUpperClauseBoundsAndIndices.clauseIndices = { 0,1,2,3,4 };
	expectedUpperClauseBoundsAndIndices.literalBounds = { 4,3,3,2,1 };

	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedLowerClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsLowerBoundsData));
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedUpperClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsUpperBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertAlreadyRecordedLowerBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, 5, -2));

	auto expectedLowerClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending);
	expectedLowerClauseBoundsAndIndices.clauseIndices = { 0,1,2,3,5,4 };
	expectedLowerClauseBoundsAndIndices.literalBounds = { -4,-3,-3,-2,-2,-1 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedLowerClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsLowerBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertNewSmallestLowerBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, 5, -5));

	auto expectedLowerClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending);
	expectedLowerClauseBoundsAndIndices.clauseIndices = { 5,0,1,2,3,4 };
	expectedLowerClauseBoundsAndIndices.literalBounds = { -5,-4,-3,-3,-2,-1 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedLowerClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsLowerBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertNewLargestLowerBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, 5, 2));

	auto expectedLowerClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending);
	expectedLowerClauseBoundsAndIndices.clauseIndices = { 0,1,2,3,4,5 };
	expectedLowerClauseBoundsAndIndices.literalBounds = { -4,-3,-3,-2,-1,2 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedLowerClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsLowerBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertAlreadyRecordedUpperBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, 5, 3));

	auto expectedUpperClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending);
	expectedUpperClauseBoundsAndIndices.clauseIndices = { 0,5,1,2,3,4 };
	expectedUpperClauseBoundsAndIndices.literalBounds = { 4,3,3,3,2,1 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedUpperClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsUpperBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertNewSmallestUpperBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, 5, -2));

	auto expectedUpperClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending);
	expectedUpperClauseBoundsAndIndices.clauseIndices = { 0,1,2,3,4,5 };
	expectedUpperClauseBoundsAndIndices.literalBounds = { 4,3,3,2,1,-2 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedUpperClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsUpperBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, InsertNewLargestUpperBound)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(insertClauseIndexAndLiteralsOrThrow(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, 5, 5));

	auto expectedUpperClauseBoundsAndIndices = AvlIntervalTreeNode::ClauseBoundsAndIndices(AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending);
	expectedUpperClauseBoundsAndIndices.clauseIndices = { 5,0,1,2,3,4 };
	expectedUpperClauseBoundsAndIndices.literalBounds = { 5,4,3,3,2,1 };
	ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expectedUpperClauseBoundsAndIndices, avlIntervalTreeNode->overlappingIntervalsUpperBoundsData));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnUpperBoundsWithNoOverlapWithSearchedForElement)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, -1, { 0,1,2,3,4 }));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnUpperBoundsWithAllUpperBoundsBeingLarger)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, 5, { }));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnUpperBoundsWithOnlySomeOverlapping)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsUpperBoundsData, 3, { 4,3,2,1 }));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnLowerBoundsWithNoOverlapWithSearchedForElement)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, 1, {0,1,2,3,4}));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnLowerBoundsWithAllLowerBoundsBeingSmaller)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, -5, {}));
}

TEST_F(AvlIntervalTreeNodeTests, DetermineOverlappingClauseIndicesBasedOnLowerBoundsWithOnlySomeOverlapping)
{
	auto avlIntervalTreeNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	const std::vector toBeInsertedClauses = {
		ClauseIndexAndLiterals(4, {-1, 1}),
		ClauseIndexAndLiterals(3, {-2, 2}),
		ClauseIndexAndLiterals(2, {-3, 3}),
		ClauseIndexAndLiterals(1, {-3, 3}),
		ClauseIndexAndLiterals(0, {-4, 4}),
	};
	ASSERT_NO_FATAL_FAILURE(insertClauseIndicesAndLiteralsOrThrow(*avlIntervalTreeNode, toBeInsertedClauses));
	ASSERT_NO_FATAL_FAILURE(assertIndicesOfClausesPotentiallyOverlappingMatchInAnyOrder(avlIntervalTreeNode->overlappingIntervalsLowerBoundsData, -3, { 4,3,2,1 }));
}