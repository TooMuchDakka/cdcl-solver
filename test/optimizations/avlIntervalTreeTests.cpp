#include "avlIntervalTreeTestsFixture.hpp"
#include "opaqueAvlIntervalTree.hpp"
#include "optimizations/blockedClauseElimination/avlIntervalTree.hpp"

using namespace avlIntervalTreeTests;

TEST_F(AvlIntervalTreeTestsFixture, InsertIntoEmptyTree)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseToInsert = 0;
	const auto literalsBoundsOfClauseToInsert = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseToInsert, literalsBoundsOfClauseToInsert));

	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = std::vector(1, avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalsBoundsOfClauseToInsert.smallestLiteral, idxOfClauseToInsert));
	expectedRootNode->upperBoundsSortedDescending = std::vector(1, avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalsBoundsOfClauseToInsert.largestLiteral, idxOfClauseToInsert));
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedClausesIndicesOfOverlappingIntervals = std::vector(1, idxOfClauseToInsert);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedClausesIndicesOfOverlappingIntervals, avlIntervalTree->getOverlappingIntervalsForLiteral(literalsBoundsOfClauseToInsert.smallestLiteral)));
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedClausesIndicesOfOverlappingIntervals, avlIntervalTree->getOverlappingIntervalsForLiteral(literalsBoundsOfClauseToInsert.largestLiteral)));
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedClausesIndicesOfOverlappingIntervals, avlIntervalTree->getOverlappingIntervalsForLiteral(expectedRootNode->key)));

	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, avlIntervalTree->getOverlappingIntervalsForLiteral(literalsBoundsOfClauseToInsert.smallestLiteral - 1)));
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, avlIntervalTree->getOverlappingIntervalsForLiteral(literalsBoundsOfClauseToInsert.largestLiteral + 1)));
}

TEST_F(AvlIntervalTreeTestsFixture, InsertOverlappingIntervalInRootNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfAlreadyExistingClausePriorToInsert = 0;
	const auto literalBoundsOfAlreadyExistingClause = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfAlreadyExistingClausePriorToInsert, literalBoundsOfAlreadyExistingClause));

	constexpr std::size_t idxOfClauseEnclosedByAlreadyExistingOne = 1;
	const auto literalBoundsOfClauseEnclosedByAlreadyExistingOne = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseEnclosedByAlreadyExistingOne, literalBoundsOfClauseEnclosedByAlreadyExistingOne));

	constexpr std::size_t idxOfClauseOverlappingExistingOneFromTheLeft = 2;
	const auto literalBoundsOfClauseOverlappingExistingOneFromTheLeft = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 1);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingExistingOneFromTheLeft, literalBoundsOfClauseOverlappingExistingOneFromTheLeft));

	constexpr std::size_t idxOfClauseOverlappingExistingOneFromTheRight = 3;
	const auto literalBoundsOfClauseOverlappingExistingOneFromTheRight = dimacs::ProblemDefinition::Clause::LiteralBounds(1, 6);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingExistingOneFromTheRight, literalBoundsOfClauseOverlappingExistingOneFromTheRight));

	const auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingExistingOneFromTheLeft.smallestLiteral, idxOfClauseOverlappingExistingOneFromTheLeft),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfAlreadyExistingClause.smallestLiteral, idxOfAlreadyExistingClausePriorToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseEnclosedByAlreadyExistingOne.smallestLiteral, idxOfClauseEnclosedByAlreadyExistingOne),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingExistingOneFromTheRight.smallestLiteral, idxOfClauseOverlappingExistingOneFromTheRight)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingExistingOneFromTheRight.largestLiteral, idxOfClauseOverlappingExistingOneFromTheRight),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfAlreadyExistingClause.largestLiteral, idxOfAlreadyExistingClausePriorToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseEnclosedByAlreadyExistingOne.largestLiteral, idxOfClauseEnclosedByAlreadyExistingOne),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingExistingOneFromTheLeft.largestLiteral, idxOfClauseOverlappingExistingOneFromTheLeft)
	};
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForSearchLeftOfMedian = { idxOfClauseOverlappingExistingOneFromTheLeft, idxOfAlreadyExistingClausePriorToInsert, idxOfClauseEnclosedByAlreadyExistingOne };
	const std::vector<std::size_t> actualOverlappingClausesForSearchLeftOfMedian = avlIntervalTree->getOverlappingIntervalsForLiteral(expectedRootNode->key - 2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForSearchLeftOfMedian, actualOverlappingClausesForSearchLeftOfMedian));

	const std::vector<std::size_t> expectedOverlappingClausesForSearchRightOfMedian = { idxOfClauseOverlappingExistingOneFromTheRight, idxOfAlreadyExistingClausePriorToInsert };
	const std::vector<std::size_t> actualOverlappingClausesForSearchRightOfMedian = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfAlreadyExistingClause.largestLiteral);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForSearchRightOfMedian, actualOverlappingClausesForSearchRightOfMedian));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralOverlappingNoNode = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfClauseOverlappingExistingOneFromTheRight.largestLiteral + 2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralOverlappingNoNode));
}

TEST_F(AvlIntervalTreeTestsFixture, InsertNonOverlappingIntervalCreatingLeftChildOfParentNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfInitialClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfInitialClauseInRoot));

	constexpr std::size_t idxOfClauseOverlappingRoot = 1;
	const auto literalBoundsOfClauseOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-5, 2);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingRoot, literalBoundsOfClauseOverlappingRoot));

	constexpr std::size_t idxOfClauseNotOverlappingRoot = 2;
	const auto literalBoundsOfClauseNotOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -6);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseNotOverlappingRoot, literalBoundsOfClauseNotOverlappingRoot));

	const auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.smallestLiteral, idxOfClauseOverlappingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfInitialClauseInRoot.smallestLiteral, idxOfClauseFormingRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfInitialClauseInRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.largestLiteral, idxOfClauseOverlappingRoot)
	};

	const auto expectedLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-8);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseNotOverlappingRoot.smallestLiteral, idxOfClauseNotOverlappingRoot) };
	expectedLeftChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseNotOverlappingRoot.largestLiteral, idxOfClauseNotOverlappingRoot) };
	expectedLeftChildOfRoot->parent = expectedRootNode;

	expectedRootNode->left = expectedLeftChildOfRoot;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralInRoot = { idxOfClauseOverlappingRoot, idxOfClauseFormingRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralInRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(-2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralInRoot, actualOverlappingClausesForLiteralInRoot));

	const std::vector<std::size_t> expectedOverlappingClauseForLiteralOnlyFoundInLeftChild = { idxOfClauseNotOverlappingRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralOnlyFoundInLeftChild = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfClauseNotOverlappingRoot.largestLiteral - 1);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseForLiteralOnlyFoundInLeftChild, actualOverlappingClausesForLiteralOnlyFoundInLeftChild));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralOverlappingNoNode = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfInitialClauseInRoot.largestLiteral + 2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralOverlappingNoNode));
}

TEST_F(AvlIntervalTreeTestsFixture, InsertNonOverlappingIntervalCreatingRightChildOfParentNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseOverlappingRoot = 1;
	const auto literalBoundsOfClauseOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(1, 5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingRoot, literalBoundsOfClauseOverlappingRoot));

	constexpr std::size_t idxOfClauseNotOverlappingRoot = 2;
	const auto literalBoundsOfClauseNotOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(5, 6);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseNotOverlappingRoot, literalBoundsOfClauseNotOverlappingRoot));

	const auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.smallestLiteral, idxOfClauseOverlappingRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.largestLiteral, idxOfClauseOverlappingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot)
	};

	const auto expectedRootNodeRightChild = std::make_shared<avl::AvlIntervalTreeNode>(6);
	ASSERT_TRUE(expectedRootNodeRightChild);
	expectedRootNodeRightChild->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNodeRightChild->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseNotOverlappingRoot.smallestLiteral, idxOfClauseNotOverlappingRoot) };
	expectedRootNodeRightChild->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseNotOverlappingRoot.largestLiteral, idxOfClauseNotOverlappingRoot) };
	expectedRootNodeRightChild->parent = expectedRootNode;

	expectedRootNode->right = expectedRootNodeRightChild;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralsOverlappingRoot = { idxOfClauseOverlappingRoot, idxOfClauseFormingRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralsOverlappingRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(4);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralsOverlappingRoot, actualOverlappingClausesForLiteralsOverlappingRoot));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralsOnlyFoundInRightChildOfRoot = { idxOfClauseNotOverlappingRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralsOnlyFoundInRightChildOfRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(6);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralsOnlyFoundInRightChildOfRoot, actualOverlappingClausesForLiteralsOnlyFoundInRightChildOfRoot));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralsOverlappingNoNodes = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfClauseNotOverlappingRoot.largestLiteral + 2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralsOverlappingNoNodes));
}

TEST_F(AvlIntervalTreeTestsFixture, InsertAlreadyExistingInterval)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseOverlappingRoot = 1;
	const auto literalBoundsOfClauseOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingRoot, literalBoundsOfClauseOverlappingRoot));

	constexpr std::size_t idxOfClauseMatchingExistingOne = 2;
	const auto literalBoundsOfClauseMatchingExistingOne = literalBoundsOfClauseOverlappingRoot;
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseMatchingExistingOne, literalBoundsOfClauseMatchingExistingOne));

	const auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.smallestLiteral, idxOfClauseOverlappingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseMatchingExistingOne.smallestLiteral, idxOfClauseMatchingExistingOne)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.largestLiteral, idxOfClauseOverlappingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseMatchingExistingOne.largestLiteral, idxOfClauseMatchingExistingOne)
	};
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralsOverlappingRoot = { idxOfClauseFormingRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralsOverlappingRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(4);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralsOverlappingRoot, actualOverlappingClausesForLiteralsOverlappingRoot));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralsMatchingRootKey = { idxOfClauseFormingRoot, idxOfClauseOverlappingRoot, idxOfClauseMatchingExistingOne };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralsMatchingRootKey = avlIntervalTree->getOverlappingIntervalsForLiteral(expectedRootNode->key);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralsMatchingRootKey, actualOverlappingClausesForLiteralsMatchingRootKey));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralsOverlappingNoNodes = avlIntervalTree->getOverlappingIntervalsForLiteral(literalBoundsOfClauseFormingRoot.largestLiteral + 2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralsOverlappingNoNodes));
}

TEST_F(AvlIntervalTreeTestsFixture, RotateLeft)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();

	constexpr std::size_t idxOfFirstClauseContainedInRoot = 0;
	const auto literalBoundsOfFirstClauseContainedInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInRoot, literalBoundsOfFirstClauseContainedInRoot));

	constexpr std::size_t idxOfSecondClauseContainedInRoot = 1;
	const auto literalBoundsOfSecondClauseContainedInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInRoot, literalBoundsOfSecondClauseContainedInRoot));

	constexpr std::size_t idxOfFirstClauseContainedInInitialRightChildOfRoot = 2;
	const auto literalBoundsOfFirstClauseContainedInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(4, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInInitialRightChildOfRoot, literalBoundsOfFirstClauseContainedInRightChildOfRoot));

	constexpr std::size_t idxOfSecondClauseContainedInInitialRightChildOfRoot = 3;
	const auto literalBoundsOfSecondClauseContainedInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(6, 8);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInInitialRightChildOfRoot, literalBoundsOfSecondClauseContainedInRightChildOfRoot));

	constexpr std::size_t idxOfFirstClauseContainedInInitialRightGrandChildOfRoot = 4;
	const auto literalBoundsOfFirstClauseContainedInRightGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(7, 10);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInInitialRightGrandChildOfRoot, literalBoundsOfFirstClauseContainedInRightGrandChildOfRoot));

	constexpr std::size_t idxOfSecondClauseContainedInInitialRightGrandChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseContainedInRightGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(8, 9);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInInitialRightGrandChildOfRoot, literalBoundsOfSecondClauseContainedInRightGrandChildOfRoot));

	auto expectedRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(6);
	ASSERT_TRUE(expectedRootNodeAfterRotation);
	expectedRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNodeAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRightChildOfRoot.smallestLiteral, idxOfFirstClauseContainedInInitialRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRightChildOfRoot.smallestLiteral, idxOfSecondClauseContainedInInitialRightChildOfRoot)
	};
	expectedRootNodeAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRightChildOfRoot.largestLiteral, idxOfSecondClauseContainedInInitialRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRightChildOfRoot.largestLiteral, idxOfFirstClauseContainedInInitialRightChildOfRoot),
	};

	auto expectedLeftChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedLeftChildOfRootNodeAfterRotation);
	expectedLeftChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRoot.smallestLiteral, idxOfFirstClauseContainedInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRoot.smallestLiteral, idxOfSecondClauseContainedInRoot)
	};
	expectedLeftChildOfRootNodeAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRoot.largestLiteral, idxOfFirstClauseContainedInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRoot.largestLiteral, idxOfSecondClauseContainedInRoot)
	};
	expectedLeftChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->left = expectedLeftChildOfRootNodeAfterRotation;

	auto expectedRightChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(9);
	ASSERT_TRUE(expectedRightChildOfRootNodeAfterRotation);
	expectedRightChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRightGrandChildOfRoot.smallestLiteral, idxOfFirstClauseContainedInInitialRightGrandChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRightGrandChildOfRoot.smallestLiteral, idxOfSecondClauseContainedInInitialRightGrandChildOfRoot)
	};
	expectedRightChildOfRootNodeAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRightGrandChildOfRoot.largestLiteral, idxOfFirstClauseContainedInInitialRightGrandChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRightGrandChildOfRoot.largestLiteral, idxOfSecondClauseContainedInInitialRightGrandChildOfRoot)
	};
	expectedRightChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->right = expectedRightChildOfRootNodeAfterRotation;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterRotation, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedIndicesOfClausesOnlyFoundInRootAfterRotation = { idxOfFirstClauseContainedInInitialRightChildOfRoot };
	const std::vector<std::size_t> actualIndicesOfClausesOnlyFoundInRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(5);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesOnlyFoundInRootAfterRotation, actualIndicesOfClausesOnlyFoundInRootAfterRotation));

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation = { idxOfSecondClauseContainedInInitialRightChildOfRoot, idxOfFirstClauseContainedInInitialRightGrandChildOfRoot, idxOfSecondClauseContainedInInitialRightGrandChildOfRoot };
	const std::vector<std::size_t> actualIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(8);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation, actualIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation));

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = { idxOfFirstClauseContainedInInitialRightChildOfRoot, idxOfFirstClauseContainedInRoot };
	const std::vector<std::size_t> actualIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(4);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation, actualIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation));

	const std::vector<std::size_t> actualIndicesOfClausesNotFoundInAnyNodeAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-100);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualIndicesOfClausesNotFoundInAnyNodeAfterRotation));
}

TEST_F(AvlIntervalTreeTestsFixture, RotateRight)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();

	constexpr std::size_t idxOfFirstClauseContainedInRoot = 0;
	const auto literalBoundsOfFirstClauseContainedInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInRoot, literalBoundsOfFirstClauseContainedInRoot));

	constexpr std::size_t idxOfSecondClauseContainedInRoot = 1;
	const auto literalBoundsOfSecondClauseContainedInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInRoot, literalBoundsOfSecondClauseContainedInRoot));

	constexpr std::size_t idxOfFirstClauseContainedInInitialLeftChildOfRoot = 2;
	const auto literalBoundsOfFirstClauseContainedInInitialLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-5, -2);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInInitialLeftChildOfRoot, literalBoundsOfFirstClauseContainedInInitialLeftChildOfRoot));

	constexpr std::size_t idxOfSecondClauseContainedInInitialLeftChildOfRoot = 3;
	const auto literalBoundsOfSecondClauseContainedInInitialLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-4, -3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInInitialLeftChildOfRoot, literalBoundsOfSecondClauseContainedInInitialLeftChildOfRoot));

	constexpr std::size_t idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot = 4;
	const auto literalBoundsOfFirstClauseContainedInInitialLeftGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot, literalBoundsOfFirstClauseContainedInInitialLeftGrandChildOfRoot));

	constexpr std::size_t idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseContainedInInitialLeftGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, -5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot, literalBoundsOfSecondClauseContainedInInitialLeftGrandChildOfRoot));

	auto expectedRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-4);
	ASSERT_TRUE(expectedRootNodeAfterRotation);
	expectedRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNodeAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInInitialLeftChildOfRoot.smallestLiteral, idxOfFirstClauseContainedInInitialLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInInitialLeftChildOfRoot.smallestLiteral, idxOfSecondClauseContainedInInitialLeftChildOfRoot)
	};
	expectedRootNodeAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInInitialLeftChildOfRoot.largestLiteral, idxOfFirstClauseContainedInInitialLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInInitialLeftChildOfRoot.largestLiteral, idxOfSecondClauseContainedInInitialLeftChildOfRoot)
	};

	auto expectedLeftChildOfRootAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-8);
	ASSERT_TRUE(expectedLeftChildOfRootAfterRotation);
	expectedLeftChildOfRootAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRootAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInInitialLeftGrandChildOfRoot.smallestLiteral, idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInInitialLeftGrandChildOfRoot.smallestLiteral, idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot)
	};
	expectedLeftChildOfRootAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInInitialLeftGrandChildOfRoot.largestLiteral, idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInInitialLeftGrandChildOfRoot.largestLiteral, idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot)
	};
	expectedLeftChildOfRootAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->left = expectedLeftChildOfRootAfterRotation;

	auto expectedRightChildOfRootAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRightChildOfRootAfterRotation);
	expectedRightChildOfRootAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildOfRootAfterRotation->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRoot.smallestLiteral, idxOfFirstClauseContainedInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRoot.smallestLiteral, idxOfSecondClauseContainedInRoot)
	};
	expectedRightChildOfRootAfterRotation->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRoot.largestLiteral, idxOfFirstClauseContainedInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRoot.largestLiteral, idxOfSecondClauseContainedInRoot)
	};
	expectedRightChildOfRootAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->right = expectedRightChildOfRootAfterRotation;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterRotation, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedIndicesOfClausesOnlyFoundInRootAfterRotation = { idxOfFirstClauseContainedInInitialLeftChildOfRoot, idxOfSecondClauseContainedInInitialLeftChildOfRoot };
	const std::vector<std::size_t> actualIndicesOfClausesOnlyFoundInRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(expectedRootNodeAfterRotation->key);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesOnlyFoundInRootAfterRotation, actualIndicesOfClausesOnlyFoundInRootAfterRotation));

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation = { idxOfFirstClauseContainedInInitialLeftChildOfRoot, idxOfFirstClauseContainedInRoot, idxOfSecondClauseContainedInRoot };
	const std::vector<std::size_t> actualIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation, actualIndicesOfClausesMatchingElementsInRootAndRightChildAfterRotation));

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = { idxOfFirstClauseContainedInInitialLeftChildOfRoot, idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot, idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot };
	const std::vector<std::size_t> actualIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-5);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation, actualIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation));

	const std::vector<std::size_t> actualIndicesOfClausesNotFoundInAnyNodeAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-100);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualIndicesOfClausesNotFoundInAnyNodeAfterRotation));
}

TEST_F(AvlIntervalTreeTestsFixture, RotateLeftRight)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRootNode = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(1, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRootNode, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRootNode = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-5, -1);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRootNode, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRootNode = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(5, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRootNode, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingLeftGrandChildOfRootNode = 3;
	const auto literalBoundsOfClauseFormingLeftGrandChildOfRootNode = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftGrandChildOfRootNode, literalBoundsOfClauseFormingLeftGrandChildOfRootNode));

	constexpr std::size_t idxOfClauseFormingRightChildOfLeftGrandChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingRightChildOfLeftGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-6, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftGrandChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftGrandChildOfRoot));

	auto expectedRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(3);
	ASSERT_TRUE(expectedRootNodeAfterRotation);
	expectedRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRootNode) };
	expectedRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRootNode) };

	auto expectedRightChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(6);
	ASSERT_TRUE(expectedRightChildOfRootNodeAfterRotation);
	expectedRightChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRootNode) };
	expectedRightChildOfRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRootNode) };
	expectedRightChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->right = expectedRightChildOfRootNodeAfterRotation;

	auto expectedLeftChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-5);
	ASSERT_TRUE(expectedLeftChildOfRootNodeAfterRotation);
	expectedLeftChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftGrandChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftGrandChildOfRoot) };
	expectedLeftChildOfRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftGrandChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftGrandChildOfRoot) };
	expectedLeftChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->left = expectedLeftChildOfRootNodeAfterRotation;

	auto expectedLeftChildFormingLeftGrandChildOfRootAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-7);
	ASSERT_TRUE(expectedLeftChildFormingLeftGrandChildOfRootAfterRotation);
	expectedLeftChildFormingLeftGrandChildOfRootAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildFormingLeftGrandChildOfRootAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfRootNode.smallestLiteral, idxOfClauseFormingLeftGrandChildOfRootNode) };
	expectedLeftChildFormingLeftGrandChildOfRootAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfRootNode.largestLiteral, idxOfClauseFormingLeftGrandChildOfRootNode) };
	expectedLeftChildFormingLeftGrandChildOfRootAfterRotation->parent = expectedLeftChildOfRootNodeAfterRotation;
	expectedLeftChildOfRootNodeAfterRotation->left = expectedLeftChildFormingLeftGrandChildOfRootAfterRotation;

	auto expectedRightChildFormingRightGrandChildOfRootAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-3);
	ASSERT_TRUE(expectedRightChildFormingRightGrandChildOfRootAfterRotation);
	expectedRightChildFormingRightGrandChildOfRootAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildFormingRightGrandChildOfRootAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRootNode) };
	expectedRightChildFormingRightGrandChildOfRootAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRootNode) };
	expectedRightChildFormingRightGrandChildOfRootAfterRotation->parent = expectedLeftChildOfRootNodeAfterRotation;
	expectedLeftChildOfRootNodeAfterRotation->right = expectedRightChildFormingRightGrandChildOfRootAfterRotation;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterRotation, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralOnlyFoundInRootAfterRotation = { idxOfClauseFormingRootNode };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralOnlyFoundInRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(1);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralOnlyFoundInRootAfterRotation, actualOverlappingClausesForLiteralOnlyFoundInRootAfterRotation));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralOnlyFoundInRightChildOfLeftChildOfRootAfterRotation = { idxOfClauseFormingLeftChildOfRootNode };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralOnlyFoundInRightChildOfLeftChildOfRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralOnlyFoundInRightChildOfLeftChildOfRootAfterRotation, actualOverlappingClausesForLiteralOnlyFoundInRightChildOfLeftChildOfRootAfterRotation));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralFoundInLeftChildOfRootAndLeftChildOfFormerAfterRotation = { idxOfClauseFormingLeftGrandChildOfRootNode, idxOfClauseFormingRightChildOfLeftGrandChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralFoundInLeftChildOfRootAndLeftChildOfFormerAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(-6);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralFoundInLeftChildOfRootAndLeftChildOfFormerAfterRotation, actualOverlappingClausesForLiteralFoundInLeftChildOfRootAndLeftChildOfFormerAfterRotation));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralFoundInNoNode = avlIntervalTree->getOverlappingIntervalsForLiteral(10);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralFoundInNoNode));
}

TEST_F(AvlIntervalTreeTestsFixture, RotateRightLeft)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-5, -3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(4, 6);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightGrandChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingRightGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 12);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightGrandChildOfRoot, literalBoundsOfClauseFormingRightGrandChildOfRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRightGrandChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOfRightGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(6, 10);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightGrandChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightGrandChildOfRoot));

	auto expectedRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNodeAfterRotation);
	expectedRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(-4);
	ASSERT_TRUE(expectedLeftChildOfRootNodeAfterRotation);
	expectedLeftChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->left = expectedLeftChildOfRootNodeAfterRotation;

	auto expectedRightChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(8);
	ASSERT_TRUE(expectedRightChildOfRootNodeAfterRotation);
	expectedRightChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightGrandChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightGrandChildOfRoot) };
	expectedRightChildOfRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightGrandChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightGrandChildOfRoot) };
	expectedRightChildOfRootNodeAfterRotation->parent = expectedRootNodeAfterRotation;
	expectedRootNodeAfterRotation->right = expectedRightChildOfRootNodeAfterRotation;

	auto expectedRightGrandChildOfRootAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(11);
	ASSERT_TRUE(expectedRightGrandChildOfRootAfterRotation);
	expectedRightGrandChildOfRootAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightGrandChildOfRootAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfRoot.smallestLiteral, idxOfClauseFormingRightGrandChildOfRoot) };
	expectedRightGrandChildOfRootAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfRoot.largestLiteral, idxOfClauseFormingRightGrandChildOfRoot) };
	expectedRightGrandChildOfRootAfterRotation->parent = expectedRightChildOfRootNodeAfterRotation;
	expectedRightChildOfRootNodeAfterRotation->right = expectedRightGrandChildOfRootAfterRotation;
	
	auto expectedLeftChildOfRightChildOfRootNodeAfterRotation = std::make_shared<avl::AvlIntervalTreeNode>(5);
	ASSERT_TRUE(expectedLeftChildOfRightChildOfRootNodeAfterRotation);
	expectedLeftChildOfRightChildOfRootNodeAfterRotation->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRightChildOfRootNodeAfterRotation->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedLeftChildOfRightChildOfRootNodeAfterRotation->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedLeftChildOfRightChildOfRootNodeAfterRotation->parent = expectedRightChildOfRootNodeAfterRotation;
	expectedRightChildOfRootNodeAfterRotation->left = expectedLeftChildOfRightChildOfRootNodeAfterRotation;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterRotation, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralOnlyFoundInRootAfterRotation = {idxOfClauseFormingRoot};
	const std::vector<std::size_t> actualOverlappingClausesForLiteralOnlyFoundInRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralOnlyFoundInRootAfterRotation, actualOverlappingClausesForLiteralOnlyFoundInRootAfterRotation));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralOnlyFoundInLeftChildOfRightChildOfRootAfterRotation = { idxOfClauseFormingLeftChildOfRightGrandChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralOnlyFoundInLeftChildOfRightChildOfRootAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(7);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralOnlyFoundInLeftChildOfRightChildOfRootAfterRotation, actualOverlappingClausesForLiteralOnlyFoundInLeftChildOfRightChildOfRootAfterRotation));

	const std::vector<std::size_t> expectedOverlappingClausesForLiteralFoundInRightChildOfRootAndRightChildOfFormerAfterRotation = { idxOfClauseFormingLeftChildOfRightGrandChildOfRoot, idxOfClauseFormingRightGrandChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClausesForLiteralFoundInRightChildOfRootAndRightChildOfFormerAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(10);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClausesForLiteralFoundInRightChildOfRootAndRightChildOfFormerAfterRotation, actualOverlappingClausesForLiteralFoundInRightChildOfRootAndRightChildOfFormerAfterRotation));

	const std::vector<std::size_t> actualOverlappingClausesForLiteralFoundInNoNode = avlIntervalTree->getOverlappingIntervalsForLiteral(100);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClausesForLiteralFoundInNoNode));
}

TEST_F(AvlIntervalTreeTestsFixture, MultipleRotations)
{
	GTEST_SKIP();
}


TEST_F(AvlIntervalTreeTestsFixture, DeleteIntervalInNodeWithMultipleIntervalsDoesOnylDeleteFormer)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, DeleteLastIntervalInNodeCausesDeletionOfNode)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfRootNode)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfNoneExistingInterval)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionInEmptyTreeHasNoEffect)
{
	GTEST_SKIP();
}


TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalNotOverlappingAnyNode)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingOnlyOneNode)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingNodeAndItsRightChild)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingNodeAndItsLeftChild)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryInEmptyTree)
{
	GTEST_SKIP();
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalMatchingMultipleNodeConnectedViaMultipleLevels)
{
	GTEST_SKIP();
}
