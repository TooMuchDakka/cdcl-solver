#include <gtest/gtest.h>
#include <optimizations/utils/avlIntervalTree.hpp>

using namespace avl;

class OpaqueAvlIntervalTree : public AvlIntervalTree {
public:
	[[nodiscard]] AvlIntervalTreeNode::ptr getRoot() const { return avlTreeRoot; }

	explicit OpaqueAvlIntervalTree(dimacs::ProblemDefinition::ptr formula)
		: AvlIntervalTree(std::move(formula)) {}
};

class AvlIntervalTreeTests : public testing::Test {
public:
	struct ClauseIndexAndLiteralPair
	{
		std::size_t index;
		std::vector<long> literals;

		explicit ClauseIndexAndLiteralPair(std::size_t index, std::initializer_list<long> literals)
			: index(index), literals(literals) {}
	};

	static void initializeFormula(dimacs::ProblemDefinition& formula, const std::initializer_list<ClauseIndexAndLiteralPair>& clauses)
	{
		for (const ClauseIndexAndLiteralPair& clause : clauses)
			ASSERT_TRUE(formula.addClause(clause.index, dimacs::ProblemDefinition::Clause(clause.literals)));
	}

	static void assertInsertOfManyClausesIsOk(AvlIntervalTree& avlIntervalTree, const std::initializer_list<ClauseIndexAndLiteralPair>& clauseIndexAndLiteralPairs)
	{
		for (const ClauseIndexAndLiteralPair& clauseIndexAndLiteralPair : clauseIndexAndLiteralPairs)
			ASSERT_TRUE(avlIntervalTree.insertClause(clauseIndexAndLiteralPair.index, dimacs::ProblemDefinition::Clause(clauseIndexAndLiteralPair.literals)));
	}

	static void assertAvlIntervalTreeNodesMatch(const AvlIntervalTreeNode& expected, const AvlIntervalTreeNode& actual)
	{
		ASSERT_EQ(expected.intervalMidPoint, actual.intervalMidPoint);
		ASSERT_EQ(expected.balancingFactor, actual.balancingFactor) << "Balancing factor missmatch in node with interval mid point " << std::to_string(expected.intervalMidPoint);
		ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expected.overlappingIntervalsLowerBoundsData, actual.overlappingIntervalsLowerBoundsData)) << " Container for lower bounds of overlapping clauses did not match in node with interval mid point " << std::to_string(expected.intervalMidPoint);
		ASSERT_NO_FATAL_FAILURE(assertClauseBoundsAndIndicesMatch(expected.overlappingIntervalsUpperBoundsData, actual.overlappingIntervalsUpperBoundsData)) << " Container for upper bounds of overlapping clauses did not match in node with interval mid point " << std::to_string(expected.intervalMidPoint);

		if (expected.parent)
		{
			ASSERT_TRUE(actual.parent);
			ASSERT_EQ(expected.intervalMidPoint, actual.intervalMidPoint);
		}
		else
		{
			ASSERT_FALSE(actual.parent);
		}

		ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodePointersMatch(expected.left, actual.left)) << "Left child missmatch in node with interval mid point " << std::to_string(expected.intervalMidPoint);
		ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodePointersMatch(expected.right, actual.right)) << "Right child missmatch in node with interval mid point " << std::to_string(expected.intervalMidPoint);
	}

	static void assertClauseBoundsAndIndicesMatch(const AvlIntervalTreeNode::ClauseBoundsAndIndices& expected, const AvlIntervalTreeNode::ClauseBoundsAndIndices& actual)
	{
		ASSERT_EQ(expected.literalBoundsSortOrder, actual.literalBoundsSortOrder);
		ASSERT_EQ(expected.clauseIndices.size(), actual.clauseIndices.size());
		ASSERT_EQ(expected.literalBounds.size(), actual.literalBounds.size());

		for (std::size_t i = 0; i < expected.clauseIndices.size(); ++i)
		{
			ASSERT_EQ(expected.clauseIndices.at(i), actual.clauseIndices.at(i));
			ASSERT_EQ(expected.literalBounds.at(i), actual.literalBounds.at(i));
		}
	}

	static void assertAvlIntervalTreeNodePointersMatch(const AvlIntervalTreeNode::ptr& expected, const AvlIntervalTreeNode::ptr& actual)
	{
		if (!expected)
		{
			ASSERT_TRUE(!actual);
			return;
		}

		ASSERT_TRUE(actual);
		ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expected, *actual));
	}

	static void assertClauseIndicesContainingLiteralMatch(const AvlIntervalTree& avlIntervalTree, long literal, const std::unordered_set<std::size_t>& expectedClauseIndices)
	{
		std::unordered_set<std::size_t> actualClauseIndices;
		ASSERT_NO_FATAL_FAILURE(actualClauseIndices = avlIntervalTree.determineIndicesOfClausesContainingLiteral(literal));
		ASSERT_EQ(expectedClauseIndices.size(), actualClauseIndices.size());

		for (const std::size_t clauseIndex : expectedClauseIndices)
			ASSERT_TRUE(actualClauseIndices.count(clauseIndex));
	}
};

TEST_F(AvlIntervalTreeTests, InsertMultipleClausesOverlappingRootNode) {
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);

	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-1, 1}),
		ClauseIndexAndLiteralPair(1, {-2, 2}),
		ClauseIndexAndLiteralPair(2, {-3, 3}),
		ClauseIndexAndLiteralPair(3, {-4, 4}),
		ClauseIndexAndLiteralPair(4, {-3, 3}),
	}));

	constexpr long expectedRootNodeIntervalMidPoint = 0;
	auto expectedRootNodeData = AvlIntervalTreeNode(expectedRootNodeIntervalMidPoint, nullptr);
	expectedRootNodeData.intervalMidPoint = expectedRootNodeIntervalMidPoint;
	expectedRootNodeData.balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRootNodeData.left = nullptr;
	expectedRootNodeData.right = nullptr;

	expectedRootNodeData.overlappingIntervalsLowerBoundsData.literalBoundsSortOrder = AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending;
	expectedRootNodeData.overlappingIntervalsLowerBoundsData.clauseIndices = {3,4,2,1,0};
	expectedRootNodeData.overlappingIntervalsLowerBoundsData.literalBounds = {-4,-3,-3,-2,-1};

	expectedRootNodeData.overlappingIntervalsUpperBoundsData.literalBoundsSortOrder = AvlIntervalTreeNode::ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending;
	expectedRootNodeData.overlappingIntervalsUpperBoundsData.clauseIndices = {3,4,2,1,0};
	expectedRootNodeData.overlappingIntervalsUpperBoundsData.literalBounds = {4,3,3,2,1};

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(expectedRootNodeData, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, InsertClauseCreatingLeftChildOfAvlTreeNode)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-1, 1}),
		ClauseIndexAndLiteralPair(1, {-2, 2}),
		ClauseIndexAndLiteralPair(2, {-6, -2}),
		ClauseIndexAndLiteralPair(3, {-4, -3})
	}));

	constexpr long expectedRootNodeIntervalMidPoint = 0;
	auto expectedRootNodeData = std::make_shared<AvlIntervalTreeNode>(expectedRootNodeIntervalMidPoint, nullptr);
	expectedRootNodeData->balancingFactor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
	expectedRootNodeData->overlappingIntervalsLowerBoundsData.clauseIndices = { 1,0 };
	expectedRootNodeData->overlappingIntervalsLowerBoundsData.literalBounds = { -2,-1 };

	expectedRootNodeData->overlappingIntervalsUpperBoundsData.clauseIndices = { 1,0 };
	expectedRootNodeData->overlappingIntervalsUpperBoundsData.literalBounds = { 2,1 };

	constexpr long expectedLeftNodeIntervalMidPoint = -4;
	auto expectedLeftNodeData = std::make_shared<AvlIntervalTreeNode>(expectedLeftNodeIntervalMidPoint, expectedRootNodeData);
	expectedLeftNodeData->overlappingIntervalsLowerBoundsData.clauseIndices = { 2,3 };
	expectedLeftNodeData->overlappingIntervalsLowerBoundsData.literalBounds = { -6,-4 };

	expectedLeftNodeData->overlappingIntervalsUpperBoundsData.clauseIndices = { 2,3 };
	expectedLeftNodeData->overlappingIntervalsUpperBoundsData.literalBounds = { -2,-3 };
	expectedRootNodeData->left = expectedLeftNodeData;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootNodeData, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, InsertClauseCreatingRightChildOfAvlTreeNode)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(1, {-2,2}),
		ClauseIndexAndLiteralPair(0, {-1,1}),
		ClauseIndexAndLiteralPair(2, {2,6}),
		ClauseIndexAndLiteralPair(3, {3,4})
	}));

	constexpr long expectedRootNodeIntervalMidPoint = 0;
	auto expectedRootNodeData = std::make_shared<AvlIntervalTreeNode>(expectedRootNodeIntervalMidPoint, nullptr);
	expectedRootNodeData->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
	expectedRootNodeData->overlappingIntervalsLowerBoundsData.clauseIndices = { 1,0 };
	expectedRootNodeData->overlappingIntervalsLowerBoundsData.literalBounds = { -2,-1 };

	expectedRootNodeData->overlappingIntervalsUpperBoundsData.clauseIndices = { 1,0 };
	expectedRootNodeData->overlappingIntervalsUpperBoundsData.literalBounds = { 2,1 };

	constexpr long expectedRightNodeIntervalMidPoint = 4;
	auto expectedRightNodeData = std::make_shared<AvlIntervalTreeNode>(expectedRightNodeIntervalMidPoint, expectedRootNodeData);
	expectedRightNodeData->overlappingIntervalsLowerBoundsData.clauseIndices = { 2,3 };
	expectedRightNodeData->overlappingIntervalsLowerBoundsData.literalBounds = { 2,3 };

	expectedRightNodeData->overlappingIntervalsUpperBoundsData.clauseIndices = { 2,3 };
	expectedRightNodeData->overlappingIntervalsUpperBoundsData.literalBounds = { 6,4 };
	expectedRootNodeData->right = expectedRightNodeData;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootNodeData, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, RotateLeft)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-3,4}),
		ClauseIndexAndLiteralPair(1, {-2,3}),
		ClauseIndexAndLiteralPair(2, {4,7}),
		ClauseIndexAndLiteralPair(3, {6,8}),
		ClauseIndexAndLiteralPair(4, {7,10}),
		ClauseIndexAndLiteralPair(5, {8,9}),
	}));

	constexpr long expectedMidPointOfRootNodePriorToRotation = 1;
	constexpr long expectedMidPointOfRightChildOfRootPriorToRotation = 6;
	constexpr long expectedMidPointOfRightGrandChildOfRootPriorToRotation = 9;
	auto expectedRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightChildOfRootPriorToRotation, nullptr);
	expectedRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 2,3 };
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 4,6 };

	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 3,2 };
	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 8,7 };


	auto expectedLeftChildAfterRotation = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRootNodePriorToRotation, expectedRootAfterRotations);
	expectedLeftChildAfterRotation->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildAfterRotation->overlappingIntervalsLowerBoundsData.clauseIndices = { 0,1 };
	expectedLeftChildAfterRotation->overlappingIntervalsLowerBoundsData.literalBounds = { -3,-2 };

	expectedLeftChildAfterRotation->overlappingIntervalsUpperBoundsData.clauseIndices = { 0,1 };
	expectedLeftChildAfterRotation->overlappingIntervalsUpperBoundsData.literalBounds = { 4,3 };


	auto expectedRightChildAfterRotation = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightGrandChildOfRootPriorToRotation, expectedRootAfterRotations);
	expectedRightChildAfterRotation->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildAfterRotation->overlappingIntervalsLowerBoundsData.clauseIndices = { 4,5 };
	expectedRightChildAfterRotation->overlappingIntervalsLowerBoundsData.literalBounds = { 7,8 };

	expectedRightChildAfterRotation->overlappingIntervalsUpperBoundsData.clauseIndices = { 4,5 };
	expectedRightChildAfterRotation->overlappingIntervalsUpperBoundsData.literalBounds = { 10,9 };

	expectedRootAfterRotations->left = expectedLeftChildAfterRotation;
	expectedRootAfterRotations->right = expectedRightChildAfterRotation;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootAfterRotations, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, RotateRight)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-3,4}),
		ClauseIndexAndLiteralPair(1, {-2,5}),
		ClauseIndexAndLiteralPair(2, {-5,-2}),
		ClauseIndexAndLiteralPair(3, {-4,-3}),
		ClauseIndexAndLiteralPair(4, {-10,-5}),
		ClauseIndexAndLiteralPair(5, {-8,-5}),
	}));

	constexpr long expectedMidPointOfRootNodePriorToRotation = 1;
	constexpr long expectedMidPointOfLeftChildOfRootPriorToRotation = -4;
	constexpr long expectedMidPointOfLeftGrandChildOfRootPriorToRotation = -8;
	auto expectedRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftChildOfRootPriorToRotation, nullptr);
	expectedRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 2,3 };
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -5,-4 };

	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 2,3 };
	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { -2,-3 };


	auto expectedLeftChildAfterRotation = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftGrandChildOfRootPriorToRotation, expectedRootAfterRotations);
	expectedLeftChildAfterRotation->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildAfterRotation->overlappingIntervalsLowerBoundsData.clauseIndices = { 4,5 };
	expectedLeftChildAfterRotation->overlappingIntervalsLowerBoundsData.literalBounds = { -10,-8 };

	expectedLeftChildAfterRotation->overlappingIntervalsUpperBoundsData.clauseIndices = { 5,4 };
	expectedLeftChildAfterRotation->overlappingIntervalsUpperBoundsData.literalBounds = { -5,-5 };


	auto expectedRightChildAfterRotation = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRootNodePriorToRotation, expectedRootAfterRotations);
	expectedRightChildAfterRotation->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildAfterRotation->overlappingIntervalsLowerBoundsData.clauseIndices = { 0,1 };
	expectedRightChildAfterRotation->overlappingIntervalsLowerBoundsData.literalBounds = { -3,-2 };

	expectedRightChildAfterRotation->overlappingIntervalsUpperBoundsData.clauseIndices = { 1,0 };
	expectedRightChildAfterRotation->overlappingIntervalsUpperBoundsData.literalBounds = { 5,4 };

	expectedRootAfterRotations->left = expectedLeftChildAfterRotation;
	expectedRootAfterRotations->right = expectedRightChildAfterRotation;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootAfterRotations, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, RotateLeftRight)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {1,4}),
		ClauseIndexAndLiteralPair(1, {-5,-1}),
		ClauseIndexAndLiteralPair(2, {5,7}),
		ClauseIndexAndLiteralPair(3, {-10,-4}),
		ClauseIndexAndLiteralPair(4, {-6,-4}),
		ClauseIndexAndLiteralPair(5, {-8,-5}),
	}));

	constexpr long expectedMidPointOfRootNodePriorToRotation = 3;
	constexpr long expectedMidPointOfLeftChildOfRootNodePriorToRotation = -3;
	constexpr long expectedMidPointOfRightChildOfRootNodePriorToRotation = 6;
	constexpr long expectedMidPointOfLeftChildOfLeftChildOfRootNodePriorToRotation = -7;
	constexpr long expectedMidPointOfRightChildOfLeftChildOfLeftChildOfRootNodePriorToRotation = -5;

	auto expectedRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRootNodePriorToRotation, nullptr);
	expectedRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 0 };
	expectedRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 1 };

	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 0 };
	expectedRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 4 };

	auto expectedLeftChildOfRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightChildOfLeftChildOfLeftChildOfRootNodePriorToRotation, expectedRootAfterRotations);
	expectedLeftChildOfRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 5,4 };
	expectedLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -8,-6 };

	expectedLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 4,5 };
	expectedLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { -4,-5 };

	auto expectedLeftChildOfLeftChildOfRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftChildOfLeftChildOfRootNodePriorToRotation, expectedLeftChildOfRootAfterRotations);
	expectedLeftChildOfLeftChildOfRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildOfLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 3 };
	expectedLeftChildOfLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -10 };

	expectedLeftChildOfLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 3 };
	expectedLeftChildOfLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { -4 };

	auto expectedRightChildOfLeftChildOfRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftChildOfRootNodePriorToRotation, expectedLeftChildOfRootAfterRotations);
	expectedRightChildOfLeftChildOfRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildOfLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 1 };
	expectedRightChildOfLeftChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -5 };

	expectedRightChildOfLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 1 };
	expectedRightChildOfLeftChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { -1 };

	auto expectedRightChildOfRootAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightChildOfRootNodePriorToRotation, expectedRootAfterRotations);
	expectedRightChildOfRootAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 2 };
	expectedRightChildOfRootAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 5 };

	expectedRightChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 2 };
	expectedRightChildOfRootAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 7 };

	expectedRootAfterRotations->left = expectedLeftChildOfRootAfterRotations;
	expectedRootAfterRotations->right = expectedRightChildOfRootAfterRotations;
	expectedLeftChildOfRootAfterRotations->left = expectedLeftChildOfLeftChildOfRootAfterRotations;
	expectedLeftChildOfRootAfterRotations->right = expectedRightChildOfLeftChildOfRootAfterRotations;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootAfterRotations, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, RotateRightLeft)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-3,4}),
		ClauseIndexAndLiteralPair(1, {-5,-3}),
		ClauseIndexAndLiteralPair(2, {4,6}),
		ClauseIndexAndLiteralPair(3, {10,12}),
		ClauseIndexAndLiteralPair(4, {11,13}),
		ClauseIndexAndLiteralPair(5, {6,10})
	}));

	constexpr long expectedMidPointOfRootNodePriorToRotation = 1;
	constexpr long expectedMidPointOfLeftChildOfRootNodePriorToRotation = -4;
	constexpr long expectedMidPointOfRightChildOfRootNodePriorToRotation = 5;
	constexpr long expectedMidPointOfRightChildOfRightChildOfRootPriorToRotation = 11;
	constexpr long expectedMidPointOfLeftChildOfRightChildOfRightChildOfRootPriorToRotation = 8;

	auto expectedRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRootNodePriorToRotation, nullptr);
	expectedRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
	expectedRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 0 };
	expectedRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -3 };

	expectedRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 0 };
	expectedRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 4 };

	auto expectedLeftChildOfRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftChildOfRootNodePriorToRotation, expectedRootNodeAfterRotations);
	expectedLeftChildOfRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 1 };
	expectedLeftChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { -5 };

	expectedLeftChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 1 };
	expectedLeftChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { -3 };


	auto expectedRightChildOfRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfLeftChildOfRightChildOfRightChildOfRootPriorToRotation, expectedRootNodeAfterRotations);
	expectedRightChildOfRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 5 };
	expectedRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 6 };

	expectedRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 5 };
	expectedRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 10 };


	auto expectedLeftChildOfRightChildOfRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightChildOfRootNodePriorToRotation, expectedRightChildOfRootNodeAfterRotations);
	expectedLeftChildOfRightChildOfRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLeftChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 2 };
	expectedLeftChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 4 };

	expectedLeftChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 2 };
	expectedLeftChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 6 };


	auto expectedRightChildOfRightChildOfRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(expectedMidPointOfRightChildOfRightChildOfRootPriorToRotation, expectedRightChildOfRootNodeAfterRotations);
	expectedRightChildOfRightChildOfRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRightChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 3,4 };
	expectedRightChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 10,11 };

	expectedRightChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 4,3 };
	expectedRightChildOfRightChildOfRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 13,12 };

	expectedRootNodeAfterRotations->left = expectedLeftChildOfRootNodeAfterRotations;
	expectedRootNodeAfterRotations->right = expectedRightChildOfRootNodeAfterRotations;
	expectedRightChildOfRootNodeAfterRotations->left = expectedLeftChildOfRightChildOfRootNodeAfterRotations;
	expectedRightChildOfRootNodeAfterRotations->right = expectedRightChildOfRightChildOfRootNodeAfterRotations;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootNodeAfterRotations, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, MultipleRotations)
{
	auto avlIntervalTree = OpaqueAvlIntervalTree(nullptr);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, {
		ClauseIndexAndLiteralPair(0, {-4,-2}),
		ClauseIndexAndLiteralPair(1, {-1,1}),
		ClauseIndexAndLiteralPair(2, {1,2}),
		ClauseIndexAndLiteralPair(3, {5,9}),
		ClauseIndexAndLiteralPair(4, {5,6}),
		ClauseIndexAndLiteralPair(5, {10,11}),
		ClauseIndexAndLiteralPair(6, {12,14}),
		ClauseIndexAndLiteralPair(7, {14,17}),
		ClauseIndexAndLiteralPair(8, {17,18}),
		ClauseIndexAndLiteralPair(9, {19,20})
	}));

	auto expectedRootNodeAfterRotations = std::make_shared<AvlIntervalTreeNode>(6, nullptr);
	expectedRootNodeAfterRotations->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
	expectedRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.clauseIndices = { 4 };
	expectedRootNodeAfterRotations->overlappingIntervalsLowerBoundsData.literalBounds = { 5 };

	expectedRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.clauseIndices = { 4 };
	expectedRootNodeAfterRotations->overlappingIntervalsUpperBoundsData.literalBounds = { 6 };

	auto expectedLChild = std::make_shared<AvlIntervalTreeNode>(0, expectedRootNodeAfterRotations);
	expectedLChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 1 };
	expectedLChild->overlappingIntervalsLowerBoundsData.literalBounds = { -1 };

	expectedLChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 1 };
	expectedLChild->overlappingIntervalsUpperBoundsData.literalBounds = { 1 };

	auto expectedRChild = std::make_shared<AvlIntervalTreeNode>(16, expectedRootNodeAfterRotations);
	expectedRChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 7 };
	expectedRChild->overlappingIntervalsLowerBoundsData.literalBounds = { 14 };

	expectedRChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 7 };
	expectedRChild->overlappingIntervalsUpperBoundsData.literalBounds = { 17 };

	auto expectedLLChild = std::make_shared<AvlIntervalTreeNode>(-3, expectedLChild);
	expectedLLChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLLChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 0 };
	expectedLLChild->overlappingIntervalsLowerBoundsData.literalBounds = { -4 };

	expectedLLChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 0 };
	expectedLLChild->overlappingIntervalsUpperBoundsData.literalBounds = { -2 };

	auto expectedLRChild = std::make_shared<AvlIntervalTreeNode>(2, expectedLChild);
	expectedLRChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLRChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 2 };
	expectedLRChild->overlappingIntervalsLowerBoundsData.literalBounds = { 1 };

	expectedLRChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 2 };
	expectedLRChild->overlappingIntervalsUpperBoundsData.literalBounds = { 2 };

	auto expectedRLChild = std::make_shared<AvlIntervalTreeNode>(11, expectedRChild);
	expectedRLChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRLChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 5 };
	expectedRLChild->overlappingIntervalsLowerBoundsData.literalBounds = { 10 };

	expectedRLChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 5 };
	expectedRLChild->overlappingIntervalsUpperBoundsData.literalBounds = { 11 };

	auto expectedRLLChild = std::make_shared<AvlIntervalTreeNode>(7, expectedRLChild);
	expectedRLLChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRLLChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 3 };
	expectedRLLChild->overlappingIntervalsLowerBoundsData.literalBounds = { 5 };

	expectedRLLChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 3 };
	expectedRLLChild->overlappingIntervalsUpperBoundsData.literalBounds = { 9 };

	auto expectedRLRChild = std::make_shared<AvlIntervalTreeNode>(13, expectedRLChild);
	expectedRLRChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRLRChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 6 };
	expectedRLRChild->overlappingIntervalsLowerBoundsData.literalBounds = { 12 };

	expectedRLRChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 6 };
	expectedRLRChild->overlappingIntervalsUpperBoundsData.literalBounds = { 14 };

	auto expectedRRChild = std::make_shared<AvlIntervalTreeNode>(18, expectedRChild);
	expectedRRChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
	expectedRRChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 8 };
	expectedRRChild->overlappingIntervalsLowerBoundsData.literalBounds = { 17 };

	expectedRRChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 8 };
	expectedRRChild->overlappingIntervalsUpperBoundsData.literalBounds = { 18 };

	auto expectedRRRChild = std::make_shared<AvlIntervalTreeNode>(20, expectedRRChild);
	expectedRRRChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRRRChild->overlappingIntervalsLowerBoundsData.clauseIndices = { 9 };
	expectedRRRChild->overlappingIntervalsLowerBoundsData.literalBounds = { 19 };

	expectedRRRChild->overlappingIntervalsUpperBoundsData.clauseIndices = { 9 };
	expectedRRRChild->overlappingIntervalsUpperBoundsData.literalBounds = { 20 };

	expectedRootNodeAfterRotations->left = expectedLChild;
	expectedRootNodeAfterRotations->right = expectedRChild;

	expectedLChild->left = expectedLLChild;
	expectedLChild->right = expectedLRChild;

	expectedRChild->left = expectedRLChild;
	expectedRChild->right = expectedRRChild;

	expectedRLChild->left = expectedRLLChild;
	expectedRLChild->right = expectedRLRChild;

	expectedRRChild->right = expectedRRRChild;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootNodeAfterRotations, *actualRootNodeData));
}

TEST_F(AvlIntervalTreeTests, DetermineIntersectingClausesInNode)
{
	auto formula = std::make_shared<dimacs::ProblemDefinition>(4, 3);
	const auto formulaClauses = {
		ClauseIndexAndLiteralPair(0, { -4,-3,2,3,4 }),
		ClauseIndexAndLiteralPair(1, { -2,-1,1,2 }),
		ClauseIndexAndLiteralPair(2, { -1,2,3,4 })
	};
	ASSERT_NO_FATAL_FAILURE(initializeFormula(*formula, formulaClauses));

	auto avlIntervalTree = OpaqueAvlIntervalTree(formula);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, formulaClauses));

	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, -1, { 1,2 }));
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, 5, { }));
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, 2, { 0,1,2 }));
}

// TODO:
TEST_F(AvlIntervalTreeTests, DetermineIntersectingClausesInNodeAndSubnodes)
{
	auto formula = std::make_shared<dimacs::ProblemDefinition>(11, 9);
	const auto formulaClauses = {
		ClauseIndexAndLiteralPair(0, {-4,-3,2,3,4}),
		ClauseIndexAndLiteralPair(1, {-2,-1,1,2}),
		ClauseIndexAndLiteralPair(2, {-1,2,3,4}),

		ClauseIndexAndLiteralPair(3, {-6,-5,-2}),
		ClauseIndexAndLiteralPair(4, {5,6,7}),

		ClauseIndexAndLiteralPair(5, {-10,-8,-6}),
		ClauseIndexAndLiteralPair(6, {-11,-7}),

		ClauseIndexAndLiteralPair(7, {-6,-5})
	};
	ASSERT_NO_FATAL_FAILURE(initializeFormula(*formula, formulaClauses));

	auto avlIntervalTree = OpaqueAvlIntervalTree(formula);
	ASSERT_NO_FATAL_FAILURE(assertInsertOfManyClausesIsOk(avlIntervalTree, formulaClauses));

	auto expectedRootNode = std::make_shared<AvlIntervalTreeNode>(0, nullptr);
	expectedRootNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
	expectedRootNode->overlappingIntervalsLowerBoundsData.clauseIndices = { 0,1,2 };
	expectedRootNode->overlappingIntervalsLowerBoundsData.literalBounds = { -4,-2,-1 };

	expectedRootNode->overlappingIntervalsUpperBoundsData.clauseIndices = { 2,0,1 };
	expectedRootNode->overlappingIntervalsUpperBoundsData.literalBounds = { 4,4,2 };

	auto expectedLChildNode = std::make_shared<AvlIntervalTreeNode>(-6, expectedRootNode);
	expectedLChildNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLChildNode->overlappingIntervalsLowerBoundsData.clauseIndices = { 5,3,7 };
	expectedLChildNode->overlappingIntervalsLowerBoundsData.literalBounds = { -10,-6,-6 };

	expectedLChildNode->overlappingIntervalsUpperBoundsData.clauseIndices = { 3,7,5 };
	expectedLChildNode->overlappingIntervalsUpperBoundsData.literalBounds = { -2,-5,-6 };

	auto expectedRChildNode = std::make_shared<AvlIntervalTreeNode>(6, expectedRootNode);
	expectedRChildNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedRChildNode->overlappingIntervalsLowerBoundsData.clauseIndices = { 4 };
	expectedRChildNode->overlappingIntervalsLowerBoundsData.literalBounds = { 5 };

	expectedRChildNode->overlappingIntervalsUpperBoundsData.clauseIndices = { 4 };
	expectedRChildNode->overlappingIntervalsUpperBoundsData.literalBounds = { 7 };

	auto expectedLLChildNode = std::make_shared<AvlIntervalTreeNode>(-8, expectedLChildNode);
	expectedLLChildNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLLChildNode->overlappingIntervalsLowerBoundsData.clauseIndices = { 6 };
	expectedLLChildNode->overlappingIntervalsLowerBoundsData.literalBounds = { -11 };

	expectedLLChildNode->overlappingIntervalsUpperBoundsData.clauseIndices = { 6 };
	expectedLLChildNode->overlappingIntervalsUpperBoundsData.literalBounds = { -7 };

	auto expectedLRChildNode = std::make_shared<AvlIntervalTreeNode>(-4, expectedLChildNode);
	expectedLRChildNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	expectedLRChildNode->overlappingIntervalsLowerBoundsData.clauseIndices = { };
	expectedLRChildNode->overlappingIntervalsLowerBoundsData.literalBounds = { };

	expectedLRChildNode->overlappingIntervalsUpperBoundsData.clauseIndices = { };
	expectedLRChildNode->overlappingIntervalsUpperBoundsData.literalBounds = { };

	expectedRootNode->left = expectedLChildNode;
	expectedRootNode->right = expectedRChildNode;
	expectedLChildNode->left = expectedLLChildNode;
	expectedLChildNode->right = expectedLRChildNode;

	const AvlIntervalTreeNode::ptr actualRootNodeData = avlIntervalTree.getRoot();
	ASSERT_NO_FATAL_FAILURE(assertAvlIntervalTreeNodesMatch(*expectedRootNode, *actualRootNodeData));

	// Partial match in some nodes
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, -2, { 1,3 }));
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, -6, { 3,5,7 }));
	// No match in any node
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, 8, { }));
	// Match in only one node
	ASSERT_NO_FATAL_FAILURE(assertClauseIndicesContainingLiteralMatch(avlIntervalTree, -10, { 5 }));
}