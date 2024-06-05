#include "avlIntervalTreeTestsFixture.hpp"
#include "opaqueAvlIntervalTree.hpp"
#include "optimizations/blockedClauseElimination/intervalTree/avlIntervalTree.hpp"

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

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = { idxOfFirstClauseContainedInRoot, idxOfFirstClauseContainedInInitialRightChildOfRoot };
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
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInInitialLeftGrandChildOfRoot.largestLiteral, idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInInitialLeftGrandChildOfRoot.largestLiteral, idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot)
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
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseContainedInRoot.largestLiteral, idxOfSecondClauseContainedInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseContainedInRoot.largestLiteral, idxOfFirstClauseContainedInRoot)
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

	const std::vector<std::size_t> expectedIndicesOfClausesMatchingElementsInRootAndLeftChildAfterRotation = { idxOfFirstClauseContainedInInitialLeftGrandChildOfRoot, idxOfSecondClauseContainedInInitialLeftGrandChildOfRoot, idxOfFirstClauseContainedInInitialLeftChildOfRoot };
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
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	// Insertion order: m, n, o, l, k , q, p , h , i, a
	constexpr long aNodeKey = -3;
	constexpr std::size_t idxOfClauseForANode = 9;
	const auto aNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(-4, -2);

	constexpr long hNodeKey = 0;
	constexpr std::size_t idxOfClauseForHNode = 7;
	const auto hNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(-1, 1);

	constexpr long iNodeKey = 2;
	constexpr std::size_t idxOfClauseForINode = 8;
	const auto iNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(1, 2);

	constexpr long lNodeKey = 7;
	constexpr std::size_t idxOfClauseForLNode = 3;
	const auto lNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(5, 9);

	constexpr long kNodeKey = 5;
	constexpr std::size_t idxOfClauseForKNode = 4;
	const auto kNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(5, 5);

	constexpr long mNodeKey = 11;
	constexpr std::size_t idxOfClauseForMNode = 0;
	const auto mNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 11);

	constexpr long nNodeKey = 13;
	constexpr std::size_t idxOfClauseForNNode = 1;
	const auto nNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);

	constexpr long oNodeKey = 16;
	constexpr std::size_t idxOfClauseForONode = 2;
	const auto oNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(14, 17);

	constexpr long pNodeKey = 18;
	constexpr std::size_t idxOfClauseForPNode = 6;
	const auto pNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(17, 18);

	constexpr long qNodeKey = 20;
	constexpr std::size_t idxOfClauseForQNode = 5;
	const auto qNodeLiteralBounds = dimacs::ProblemDefinition::Clause::LiteralBounds(19, 20);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForMNode, mNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForNNode, nNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForONode, oNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForLNode, lNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForKNode, kNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForQNode, qNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForPNode, pNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForHNode, hNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForINode, iNodeLiteralBounds));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseForANode, aNodeLiteralBounds));

	auto expectedANode = std::make_shared<avl::AvlIntervalTreeNode>(aNodeKey);
	ASSERT_TRUE(expectedANode);
	expectedANode->key = aNodeKey;
	expectedANode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedANode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(aNodeLiteralBounds.smallestLiteral, idxOfClauseForANode) };
	expectedANode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(aNodeLiteralBounds.largestLiteral, idxOfClauseForANode) };

	auto expectedHNode = std::make_shared<avl::AvlIntervalTreeNode>(hNodeKey);
	ASSERT_TRUE(expectedHNode);
	expectedHNode->key = hNodeKey;
	expectedHNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedHNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(hNodeLiteralBounds.smallestLiteral, idxOfClauseForHNode) };
	expectedHNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(hNodeLiteralBounds.largestLiteral, idxOfClauseForHNode) };

	auto expectedINode = std::make_shared<avl::AvlIntervalTreeNode>(iNodeKey);
	ASSERT_TRUE(expectedINode);
	expectedINode->key = iNodeKey;
	expectedINode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedINode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(iNodeLiteralBounds.smallestLiteral, idxOfClauseForINode) };
	expectedINode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(iNodeLiteralBounds.largestLiteral, idxOfClauseForINode) };

	auto expectedLNode = std::make_shared<avl::AvlIntervalTreeNode>(lNodeKey);
	ASSERT_TRUE(expectedLNode);
	expectedLNode->key = lNodeKey;
	expectedLNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(lNodeLiteralBounds.smallestLiteral, idxOfClauseForLNode) };
	expectedLNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(lNodeLiteralBounds.largestLiteral, idxOfClauseForLNode) };

	auto expectedKNode = std::make_shared<avl::AvlIntervalTreeNode>(kNodeKey);
	ASSERT_TRUE(expectedKNode);
	expectedKNode->key = kNodeKey;
	expectedKNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedKNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(kNodeLiteralBounds.smallestLiteral, idxOfClauseForKNode) };
	expectedKNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(kNodeLiteralBounds.largestLiteral, idxOfClauseForKNode) };

	auto expectedMNode = std::make_shared<avl::AvlIntervalTreeNode>(mNodeKey);
	ASSERT_TRUE(expectedMNode);
	expectedMNode->key = mNodeKey;
	expectedMNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedMNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(mNodeLiteralBounds.smallestLiteral, idxOfClauseForMNode) };
	expectedMNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(mNodeLiteralBounds.largestLiteral, idxOfClauseForMNode) };

	auto expectedNNode = std::make_shared<avl::AvlIntervalTreeNode>(nNodeKey);
	ASSERT_TRUE(expectedNNode);
	expectedNNode->key = nNodeKey;
	expectedNNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(nNodeLiteralBounds.smallestLiteral, idxOfClauseForNNode) };
	expectedNNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(nNodeLiteralBounds.largestLiteral, idxOfClauseForNNode) };

	auto expectedONode = std::make_shared<avl::AvlIntervalTreeNode>(oNodeKey);
	ASSERT_TRUE(expectedONode);
	expectedONode->key = oNodeKey;
	expectedONode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedONode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(oNodeLiteralBounds.smallestLiteral, idxOfClauseForONode) };
	expectedONode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(oNodeLiteralBounds.largestLiteral, idxOfClauseForONode) };

	auto expectedPNode = std::make_shared<avl::AvlIntervalTreeNode>(pNodeKey);
	ASSERT_TRUE(expectedPNode);
	expectedPNode->key = pNodeKey;
	expectedPNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedPNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(pNodeLiteralBounds.smallestLiteral, idxOfClauseForPNode) };
	expectedPNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(pNodeLiteralBounds.largestLiteral, idxOfClauseForPNode) };

	auto expectedQNode = std::make_shared<avl::AvlIntervalTreeNode>(qNodeKey);
	ASSERT_TRUE(expectedQNode);
	expectedQNode->key = qNodeKey;
	expectedQNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedQNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(qNodeLiteralBounds.smallestLiteral, idxOfClauseForQNode) };
	expectedQNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(qNodeLiteralBounds.largestLiteral, idxOfClauseForQNode) };

	expectedNNode->left = expectedINode;
	expectedNNode->right = expectedPNode;

	expectedINode->left = expectedHNode;
	expectedINode->right = expectedLNode;
	expectedINode->parent = expectedNNode;

	expectedHNode->left = expectedANode;
	expectedHNode->parent = expectedINode;

	expectedANode->parent = expectedHNode;

	expectedLNode->left = expectedKNode;
	expectedLNode->right = expectedMNode;
	expectedLNode->parent = expectedINode;

	expectedKNode->parent = expectedLNode;
	expectedMNode->parent = expectedLNode;

	expectedPNode->left = expectedONode;
	expectedPNode->right = expectedQNode;
	expectedPNode->parent = expectedNNode;

	expectedONode->parent = expectedPNode;
	expectedQNode->parent = expectedPNode;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedNNode, avlIntervalTree->getRootNode()));

	constexpr long literalOverlappingLAndKNode = 5;
	const std::vector<std::size_t> expectedIndicesClausesOfLiteralOverlappingMultipleNodes = { idxOfClauseForKNode, idxOfClauseForLNode };
	const std::vector<std::size_t> actualIndicesOfClausesOfLiteralOverlappingMultipleNodes = avlIntervalTree->getOverlappingIntervalsForLiteral(literalOverlappingLAndKNode);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesClausesOfLiteralOverlappingMultipleNodes, actualIndicesOfClausesOfLiteralOverlappingMultipleNodes));

	constexpr long literalOverlappingOnlyANode = -3;
	const std::vector<std::size_t> expectedIndicesOfClausesOfLiteralOverlappingOnlyLeftmostNodeAfterRotation = { idxOfClauseForANode };
	const std::vector<std::size_t>  actualIndicesOfClausesOfLiteralOverlappingOnlyLeftmostNodeAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(literalOverlappingOnlyANode);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesOfLiteralOverlappingOnlyLeftmostNodeAfterRotation, actualIndicesOfClausesOfLiteralOverlappingOnlyLeftmostNodeAfterRotation));

	constexpr long literalOverlappingNoNode = -20;
	const std::vector<std::size_t> actualIndicesOfClausesForLiteralOverlappingNoNodes = avlIntervalTree->getOverlappingIntervalsForLiteral(literalOverlappingNoNode);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualIndicesOfClausesForLiteralOverlappingNoNodes));

	constexpr long literalOverlappingPAndONode = 17;
	const std::vector<std::size_t> expectedIndicesOfClausesOfLiteralOverlappingMultipleNodesInRightSubtreeAfterRotation = { idxOfClauseForONode, idxOfClauseForPNode };
	const std::vector<std::size_t> actualIndicesOfClausesOfLiteralOverlappingMultipleNodesInRightSubtreeAfterRotation = avlIntervalTree->getOverlappingIntervalsForLiteral(literalOverlappingPAndONode);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedIndicesOfClausesOfLiteralOverlappingMultipleNodesInRightSubtreeAfterRotation, actualIndicesOfClausesOfLiteralOverlappingMultipleNodesInRightSubtreeAfterRotation));
}


TEST_F(AvlIntervalTreeTestsFixture, DeleteIntervalInNodeWithMultipleIntervalsDoesOnylMatchingBoundFromNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfFirstClauseToInsert = 0;
	const auto literalBoundsOfFirstClauseToInsert = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);

	constexpr std::size_t idxOfSecondClauseToInsert = 1;
	const auto literalBoundsOfSecondClauseToInsert = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 1);

	constexpr std::size_t idxOfThirdClauseToInsert = 2;
	const auto literalBoundsOfThirdClauseToInsert = dimacs::ProblemDefinition::Clause::LiteralBounds(-1, 3);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFirstClauseToInsert, literalBoundsOfFirstClauseToInsert));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseToInsert, literalBoundsOfSecondClauseToInsert));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseToInsert, literalBoundsOfThirdClauseToInsert));

	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseToInsert.smallestLiteral, idxOfFirstClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseToInsert.smallestLiteral, idxOfSecondClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseToInsert.smallestLiteral, idxOfThirdClauseToInsert)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseToInsert.largestLiteral, idxOfThirdClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfFirstClauseToInsert.largestLiteral, idxOfFirstClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseToInsert.largestLiteral, idxOfSecondClauseToInsert)
	};
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfFirstClauseToInsert, literalBoundsOfFirstClauseToInsert));
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseToInsert.smallestLiteral, idxOfSecondClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseToInsert.smallestLiteral, idxOfThirdClauseToInsert)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseToInsert.largestLiteral, idxOfThirdClauseToInsert),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseToInsert.largestLiteral, idxOfSecondClauseToInsert)
	};
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeleteLastIntervalInNodeCausesDeletionOfNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-6, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseOverlappingLeftChildOfRoot = 2;
	const auto literalBoundsOfClauseOverlappingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-7, -5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingLeftChildOfRoot, literalBoundsOfClauseOverlappingLeftChildOfRoot));

	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftNodeOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-5);
	ASSERT_TRUE(expectedLeftNodeOfRoot);
	expectedLeftNodeOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingLeftChildOfRoot.smallestLiteral, idxOfClauseOverlappingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot)
	};
	expectedLeftNodeOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingLeftChildOfRoot.largestLiteral, idxOfClauseOverlappingLeftChildOfRoot)
	};
	expectedLeftNodeOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftNodeOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseOverlappingLeftChildOfRoot, literalBoundsOfClauseOverlappingLeftChildOfRoot));
	expectedLeftNodeOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftNodeOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->left = nullptr;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	expectedRootNode = nullptr;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfRootNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-5, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(8, 10);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightGrandChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingRightGrandChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 11);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightGrandChildOfRoot, literalBoundsOfClauseFormingRightGrandChildOfRoot));

	constexpr std::size_t idxOfClauseFormingInorderSuccessorOfRoot = 4;
	const auto literalBoundsOfClauseFormingInorderSuccessorOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(4, 5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingInorderSuccessorOfRoot, literalBoundsOfClauseFormingInorderSuccessorOfRoot));

	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-5);
	expectedLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	auto expectedRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(9);
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	auto expectedInorderSuccessorOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(5);
	expectedInorderSuccessorOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedInorderSuccessorOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingInorderSuccessorOfRoot.smallestLiteral, idxOfClauseFormingInorderSuccessorOfRoot) };
	expectedInorderSuccessorOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingInorderSuccessorOfRoot.largestLiteral, idxOfClauseFormingInorderSuccessorOfRoot) };
	expectedInorderSuccessorOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->left = expectedInorderSuccessorOfRoot;

	auto expectedRightGrandChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(11);
	expectedRightGrandChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightGrandChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfRoot.smallestLiteral, idxOfClauseFormingRightGrandChildOfRoot) };
	expectedRightGrandChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfRoot.largestLiteral, idxOfClauseFormingRightGrandChildOfRoot) };
	expectedRightGrandChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->right = expectedRightGrandChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	expectedInorderSuccessorOfRoot->parent = nullptr;
	expectedInorderSuccessorOfRoot->left = expectedLeftChildOfRoot;
	expectedInorderSuccessorOfRoot->right = expectedRightChildOfRoot;
	expectedLeftChildOfRoot->parent = expectedInorderSuccessorOfRoot;
	expectedRightChildOfRoot->parent = expectedInorderSuccessorOfRoot;
	expectedRightChildOfRoot->left = nullptr;

	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedInorderSuccessorOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode = expectedInorderSuccessorOfRoot;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfNotExistingInterval)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 5);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseOverlappingRoot = 1;
	const auto literalBoundsOfClauseOverlappingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-2, 2);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseOverlappingRoot, literalBoundsOfClauseOverlappingRoot));

	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.smallestLiteral, idxOfClauseOverlappingRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseOverlappingRoot.largestLiteral, idxOfClauseOverlappingRoot)
	};
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const long missmatchingIntervalLowerBound = literalBoundsOfClauseFormingRoot.smallestLiteral - 1;
	const long missmatchingIntervalUpperBound = literalBoundsOfClauseFormingRoot.largestLiteral - 1;
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::NotFound, avlIntervalTree->removeClause(idxOfClauseFormingRoot, dimacs::ProblemDefinition::Clause::LiteralBounds(missmatchingIntervalLowerBound, missmatchingIntervalLowerBound)));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::NotFound, avlIntervalTree->removeClause(idxOfClauseFormingRoot, dimacs::ProblemDefinition::Clause::LiteralBounds(missmatchingIntervalUpperBound, missmatchingIntervalUpperBound)));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::NotFound, avlIntervalTree->removeClause(idxOfClauseOverlappingRoot, literalBoundsOfClauseFormingRoot));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionInEmptyTreeHasNoEffect)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(nullptr, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::NotFound, avlIntervalTree->removeClause(0, dimacs::ProblemDefinition::Clause::LiteralBounds(-1, 1)));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfNodeWithNoChildrenRequiringNoRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-7, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(3, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightSubNodeOfRightChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(6, 8);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightSubNodeOfRightChildOfRoot, literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-6);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	auto expectedRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(5);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	auto expectedRightSubNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(7);
	ASSERT_TRUE(expectedRightSubNodeOfRightChildOfRoot);
	expectedRightSubNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightSubNodeOfRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightSubNodeOfRightChildOfRoot) };
	expectedRightSubNodeOfRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot.largestLiteral, idxOfClauseFormingRightSubNodeOfRightChildOfRoot) };
	expectedRightSubNodeOfRightChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->right = expectedRightSubNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightSubNodeOfRightChildOfRoot, literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot));

	expectedRightChildOfRoot->right = nullptr;
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfNodeWithOneChildBeingLeftNodeRequiringNoRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-7, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(3, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingLeftSubNodeOfRightChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingLeftSubNodeOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(2, 4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftSubNodeOfRightChildOfRoot, literalBoundsOfClauseFormingLeftSubNodeOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-6);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	auto expectedRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(5);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	auto expectedLeftSubNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(3);
	ASSERT_TRUE(expectedLeftSubNodeOfRightChildOfRoot);
	expectedLeftSubNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftSubNodeOfRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubNodeOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftSubNodeOfRightChildOfRoot) };
	expectedLeftSubNodeOfRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubNodeOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftSubNodeOfRightChildOfRoot) };
	expectedLeftSubNodeOfRightChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->left = expectedLeftSubNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	expectedLeftSubNodeOfRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedLeftSubNodeOfRightChildOfRoot;
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	expectedRightChildOfRoot->left = nullptr;
	expectedRightChildOfRoot->parent = nullptr;
	expectedRightChildOfRoot->right = nullptr;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionOfNodeWithOneChildBeingRightNodeRequiringNoRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-7, -4);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(3, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightSubNodeOfRightChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(6, 8);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightSubNodeOfRightChildOfRoot, literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(0);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNode->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNode->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(-6);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	auto expectedRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(5);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	auto expectedRightSubNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(7);
	ASSERT_TRUE(expectedRightSubNodeOfRightChildOfRoot);
	expectedRightSubNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightSubNodeOfRightChildOfRoot->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightSubNodeOfRightChildOfRoot) };
	expectedRightSubNodeOfRightChildOfRoot->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubNodeOfRightChildOfRoot.largestLiteral, idxOfClauseFormingRightSubNodeOfRightChildOfRoot) };
	expectedRightSubNodeOfRightChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->right = expectedRightSubNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	expectedRightSubNodeOfRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightSubNodeOfRightChildOfRoot;
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	expectedRightChildOfRoot->right = nullptr;
	expectedRightChildOfRoot->parent = nullptr;
	expectedRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));
}

/*
 *See further https://www.cs.emory.edu/~cheung/Courses/253/Syllabus/Trees/AVL-delete.html
 */
TEST_F(AvlIntervalTreeTestsFixture, DeletionRequiringRotateLeftOperationForRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(44, 45);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(27, 32);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(55, 57);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightNodeOfRightChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingRightNodeOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(58, 60);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfRightChildOfRoot, literalBoundsOfClauseFormingRightNodeOfRightChildOfRoot));


	auto expectedRootNodePriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(45);
	ASSERT_TRUE(expectedRootNodePriorToDeletion);
	expectedRootNodePriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNodePriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNodePriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(30);
	ASSERT_TRUE(expectedLeftNodeOfRootPriorToDeletion);
	expectedLeftNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftNodeOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->left = expectedLeftNodeOfRootPriorToDeletion;

	auto expectedRightNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(56);
	ASSERT_TRUE(expectedRightNodeOfRootPriorToDeletion);
	expectedRightNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRightNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightNodeOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->right = expectedRightNodeOfRootPriorToDeletion;

	auto expectedRightNodeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(59);
	ASSERT_TRUE(expectedRightNodeOfRightChildOfRootPriorToDeletion);
	expectedRightNodeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightNodeOfRightChildOfRoot) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightChildOfRoot.largestLiteral, idxOfClauseFormingRightNodeOfRightChildOfRoot) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->parent = expectedRightNodeOfRootPriorToDeletion;
	expectedRightNodeOfRootPriorToDeletion->right = expectedRightNodeOfRightChildOfRootPriorToDeletion;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodePriorToDeletion, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	avl::AvlIntervalTreeNode::ptr expectedRootNodeAfterDeletion = expectedRightNodeOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfRootAfterDeletion = expectedRootNodePriorToDeletion;
	expectedLeftNodeOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedLeftNodeOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfRootAfterDeletion->right = nullptr;
	expectedRootNodeAfterDeletion->left = expectedLeftNodeOfRootAfterDeletion;

	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfRootAfterDeletion = expectedRightNodeOfRightChildOfRootPriorToDeletion;
	expectedRightNodeOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedRightNodeOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfRootAfterDeletion->right = nullptr;
	expectedRootNodeAfterDeletion->right = expectedRightNodeOfRootAfterDeletion;

	expectedRootNodeAfterDeletion->parent = nullptr;
	expectedRootNodeAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterDeletion, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionRequiringRotateRightOperationForRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(44, 45);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(27, 32);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRoot));

	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(55, 57);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	constexpr std::size_t idxOfClauseFormingLeftNodeOfLeftChildOfRoot = 3;
	const auto literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 15);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfLeftChildOfRoot, literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRoot));


	auto expectedRootNodePriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(45);
	ASSERT_TRUE(expectedRootNodePriorToDeletion);
	expectedRootNodePriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootNodePriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot) };
	expectedRootNodePriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot) };

	auto expectedLeftNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(30);
	ASSERT_TRUE(expectedLeftNodeOfRootPriorToDeletion);
	expectedLeftNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedLeftNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot) };
	expectedLeftNodeOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->left = expectedLeftNodeOfRootPriorToDeletion;

	auto expectedLeftNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(13);
	ASSERT_TRUE(expectedLeftNodeOfLeftChildOfRootPriorToDeletion);
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingLeftNodeOfLeftChildOfRoot) };
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingLeftNodeOfLeftChildOfRoot) };
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftNodeOfRootPriorToDeletion;
	expectedLeftNodeOfRootPriorToDeletion->left = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedRightNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(56);
	ASSERT_TRUE(expectedRightNodeOfRootPriorToDeletion);
	expectedRightNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot) };
	expectedRightNodeOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->right = expectedRightNodeOfRootPriorToDeletion;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodePriorToDeletion, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));

	avl::AvlIntervalTreeNode::ptr expectedRootNodeAfterDeletion = expectedLeftNodeOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfRootAfterDeletion = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftNodeOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedLeftNodeOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfRootAfterDeletion->right = nullptr;
	expectedRootNodeAfterDeletion->left = expectedLeftNodeOfRootAfterDeletion;

	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfRootAfterDeletion = expectedRootNodePriorToDeletion;
	expectedRightNodeOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedRightNodeOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfRootAfterDeletion->right = nullptr;
	expectedRootNodeAfterDeletion->right = expectedRightNodeOfRootAfterDeletion;

	expectedRootNodeAfterDeletion->parent = nullptr;
	expectedRootNodeAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterDeletion, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionRequiringRotateRightLeftOperationForRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRootPriorToDeletion = 0;
	const auto literalBoundsOfClauseFormingRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(42, 44);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRootPriorToDeletion, literalBoundsOfClauseFormingRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRootPriorToDeletion = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(17, 20);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightChildOfRootPriorToDeletion = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(78, 80);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion = 3;
	const auto literalBoundsOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(50, 52);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion = 4;
	const auto literalBoundsOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(88, 90);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion = 5;
	const auto literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(32, 34);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = 6;
	const auto literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(48, 49);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = 7;
	const auto literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(53, 55);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion));

	auto expectedRootNodePriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(43);
	ASSERT_TRUE(expectedRootNodePriorToDeletion);
	expectedRootNodePriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedRootNodePriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRootPriorToDeletion) };
	expectedRootNodePriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.largestLiteral, idxOfClauseFormingRootPriorToDeletion) };

	auto expectedLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(19);
	ASSERT_TRUE(expectedLeftChildOfRootPriorToDeletion);
	expectedLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftChildOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->left = expectedLeftChildOfRootPriorToDeletion;

	auto expectedRightNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(33);
	ASSERT_TRUE(expectedRightNodeOfLeftChildOfRootPriorToDeletion);
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftChildOfRootPriorToDeletion;
	expectedLeftChildOfRootPriorToDeletion->right = expectedRightNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(79);
	ASSERT_TRUE(expectedRightChildOfRootPriorToDeletion);
	expectedRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightChildOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->right = expectedRightChildOfRootPriorToDeletion;

	auto expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(51);
	ASSERT_TRUE(expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion);
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion) };
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftSubtreeRootOfRightChildOfRootPriorToDeletion) };
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->parent = expectedRightChildOfRootPriorToDeletion;
	expectedRightChildOfRootPriorToDeletion->left = expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion;

	auto expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(89);
	ASSERT_TRUE(expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion);
	expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion) };
	expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightSubtreeRootOfRightChildOfRootPriorToDeletion) };
	expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion->parent = expectedRightChildOfRootPriorToDeletion;
	expectedRightChildOfRootPriorToDeletion->right = expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion;

	auto expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(49);
	ASSERT_TRUE(expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion);
	expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->parent = expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion;
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->left = expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion;

	auto expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(54);
	ASSERT_TRUE(expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion);
	expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion->parent = expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion;
	expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion->right = expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodePriorToDeletion, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion));

	avl::AvlIntervalTreeNode::ptr expectedRootNodeAfterDeletion = expectedLeftSubtreeRootOfRightChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfRootAfterDeletion = expectedRootNodePriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfLeftChildOfRootAfterDeletion = expectedLeftChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfLeftChildOfRootAfterDeletion = expectedLeftNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion;

	avl::AvlIntervalTreeNode::ptr expectedRightChildOfRootAfterDeletion = expectedRightChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfRightChildOfRootAfterDeletion = expectedRightNodeOfRightSubtreeOfRightChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfRightChildOfRootAfterDeletion = expectedRightSubtreeRootOfRightChildOfRootPriorToDeletion;

	expectedLeftNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->right = nullptr;

	expectedRightNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->right = nullptr;

	expectedLeftChildOfRootAfterDeletion->left = expectedLeftNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->right = expectedRightNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;


	expectedLeftNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->right = nullptr;

	expectedRightNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightChildOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfRightChildOfRootAfterDeletion->right = nullptr;

	expectedRightChildOfRootAfterDeletion->left = expectedLeftNodeOfRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->right = expectedRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	expectedRootNodeAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNodeAfterDeletion->left = expectedLeftChildOfRootAfterDeletion;
	expectedRootNodeAfterDeletion->right = expectedRightChildOfRootAfterDeletion;
	expectedRootNodeAfterDeletion->parent = nullptr;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterDeletion, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionRequiringRotateLeftRightOperationForRebalancing)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr std::size_t idxOfClauseFormingRootPriorToDeletion = 0;
	const auto literalBoundsOfClauseFormingRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(42, 44);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRootPriorToDeletion, literalBoundsOfClauseFormingRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftChildOfRootPriorToDeletion = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(30, 32);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightChildOfRootPriorToDeletion = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(55, 57);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion = 3;
	const auto literalBoundsOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(28, 29);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion = 4;
	const auto literalBoundsOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(35, 37);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion = 5;
	const auto literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(48, 50);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = 6;
	const auto literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(33, 34);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = 7;
	const auto literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(37, 39);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion));

	auto expectedRootNodePriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(43);
	ASSERT_TRUE(expectedRootNodePriorToDeletion);
	expectedRootNodePriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootNodePriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRootPriorToDeletion) };
	expectedRootNodePriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.largestLiteral, idxOfClauseFormingRootPriorToDeletion) };

	auto expectedLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(31);
	ASSERT_TRUE(expectedLeftChildOfRootPriorToDeletion);
	expectedLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftChildOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->left = expectedLeftChildOfRootPriorToDeletion;


	auto expectedRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(56);
	ASSERT_TRUE(expectedRightChildOfRootPriorToDeletion);
	expectedRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightChildOfRootPriorToDeletion->parent = expectedRootNodePriorToDeletion;
	expectedRootNodePriorToDeletion->right = expectedRightChildOfRootPriorToDeletion;

	auto expectedRightNodeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(49);
	ASSERT_TRUE(expectedRightNodeOfRightChildOfRootPriorToDeletion);
	expectedRightNodeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->parent = expectedRightChildOfRootPriorToDeletion;
	expectedRightChildOfRootPriorToDeletion->left = expectedRightNodeOfRightChildOfRootPriorToDeletion;

	auto expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(29);
	ASSERT_TRUE(expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion);
	expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion) };
	expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftSubtreeRootOfLeftChildOfRootPriorToDeletion) };
	expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion->parent = expectedLeftChildOfRootPriorToDeletion;
	expectedLeftChildOfRootPriorToDeletion->left = expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion;

	auto expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(36);
	ASSERT_TRUE(expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion);
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion) };
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightSubtreeRootOfLeftChildOfRootPriorToDeletion) };
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->parent = expectedLeftChildOfRootPriorToDeletion;
	expectedLeftChildOfRootPriorToDeletion->right = expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion;

	auto expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(34);
	ASSERT_TRUE(expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion);
	expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->parent = expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion;
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->left = expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion;

	auto expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(38);
	ASSERT_TRUE(expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion);
	expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion->parent = expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion;
	expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion->right = expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodePriorToDeletion, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion));

	avl::AvlIntervalTreeNode::ptr expectedRootNodeAfterDeletion = expectedRightSubtreeRootOfLeftChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfRootAfterDeletion = expectedLeftChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfLeftChildOfRootAfterDeletion = expectedLeftSubtreeRootOfLeftChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfLeftChildOfRootAfterDeletion = expectedLeftNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion;

	avl::AvlIntervalTreeNode::ptr expectedRightChildOfRootAfterDeletion = expectedRootNodePriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfRightChildOfRootAfterDeletion = expectedRightNodeOfRightSubtreeOfLeftChildOfRootPriorToDeletion;
	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfRightChildOfRootAfterDeletion = expectedRightChildOfRootPriorToDeletion;

	expectedLeftNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->right = nullptr;

	expectedRightNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->right = nullptr;

	expectedLeftChildOfRootAfterDeletion->left = expectedLeftNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->right = expectedRightNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;


	expectedLeftNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->left = nullptr;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->right = nullptr;

	expectedRightNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightChildOfRootAfterDeletion->left = nullptr;
	expectedRightNodeOfRightChildOfRootAfterDeletion->right = nullptr;

	expectedRightChildOfRootAfterDeletion->left = expectedLeftNodeOfRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->right = expectedRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->parent = expectedRootNodeAfterDeletion;
	expectedRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	expectedRootNodeAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNodeAfterDeletion->left = expectedLeftChildOfRootAfterDeletion;
	expectedRootNodeAfterDeletion->right = expectedRightChildOfRootAfterDeletion;
	expectedRootNodeAfterDeletion->parent = nullptr;
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNodeAfterDeletion, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, DeletionRequiringRotationsAtMultipleLevels)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	// Level 0
	constexpr std::size_t idxOfClauseFormingRootPriorToDeletion = 0;
	const auto literalBoundsOfClauseFormingRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(50, 52);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRootPriorToDeletion, literalBoundsOfClauseFormingRootPriorToDeletion));

	// Level 1
	constexpr std::size_t idxOfClauseFormingLeftChildOfRootPriorToDeletion = 1;
	const auto literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(25, 27);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightChildOfRootPriorToDeletion = 2;
	const auto literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(75, 77);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion));

	// Level 2
	constexpr std::size_t idxOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion = 3;
	const auto literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 12);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion = 4;
	const auto literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(30, 32);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion = 5;
	const auto literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(60, 62);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion = 7;
	const auto literalBoundsOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(80, 82);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion));

	// Level 3
	constexpr std::size_t idxOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion = 8;
	const auto literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(5, 7);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion = 9;
	const auto literalBoundsOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(15, 17);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion, literalBoundsOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion = 10;
	const auto literalBoundsOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(27, 29);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion));

	constexpr std::size_t idxOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion = 11;
	const auto literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(55, 57);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion, literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion));

	// Level 4
	constexpr std::size_t idxOfClauseFormingLeftmostNodeOfTreePriorToDeletion = 12;
	const auto literalBoundsOfClauseFormingLeftmostNodeOfTreePriorToDeletion = dimacs::ProblemDefinition::Clause::LiteralBounds(1, 3);
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftmostNodeOfTreePriorToDeletion, literalBoundsOfClauseFormingLeftmostNodeOfTreePriorToDeletion));


	// Level 0
	auto expectedRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(51);
	ASSERT_TRUE(expectedRootPriorToDeletion);
	expectedRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRootPriorToDeletion) };
	expectedRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRootPriorToDeletion.largestLiteral, idxOfClauseFormingRootPriorToDeletion) };
	expectedRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;

	// Level 1
	auto expectedLeftNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(26);
	ASSERT_TRUE(expectedLeftNodeOfRootPriorToDeletion);
	expectedLeftNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedLeftNodeOfRootPriorToDeletion->parent = expectedRootPriorToDeletion;
	expectedRootPriorToDeletion->left = expectedLeftNodeOfRootPriorToDeletion;

	auto expectedRightNodeOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(76);
	ASSERT_TRUE(expectedRightNodeOfRootPriorToDeletion);
	expectedRightNodeOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRightNodeOfRootPriorToDeletion->parent = expectedRootPriorToDeletion;
	expectedRootPriorToDeletion->right = expectedRightNodeOfRootPriorToDeletion;

	// Level 2
	auto expectedLeftNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(11);
	ASSERT_TRUE(expectedLeftNodeOfLeftChildOfRootPriorToDeletion);
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftNodeOfLeftChildOfRootPriorToDeletion) };
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftNodeOfRootPriorToDeletion;
	expectedLeftNodeOfRootPriorToDeletion->left = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedRightNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(31);
	ASSERT_TRUE(expectedRightNodeOfLeftChildOfRootPriorToDeletion);
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightNodeOfLeftChildOfRootPriorToDeletion) };
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftNodeOfRootPriorToDeletion;
	expectedLeftNodeOfRootPriorToDeletion->right = expectedRightNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedLeftNodeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(61);
	ASSERT_TRUE(expectedLeftNodeOfRightChildOfRootPriorToDeletion);
	expectedLeftNodeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftNodeOfRightChildOfRootPriorToDeletion) };
	expectedLeftNodeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedLeftNodeOfRightChildOfRootPriorToDeletion->parent = expectedRightNodeOfRootPriorToDeletion;
	expectedRightNodeOfRootPriorToDeletion->left = expectedLeftNodeOfRightChildOfRootPriorToDeletion;

	auto expectedRightNodeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(81);
	ASSERT_TRUE(expectedRightNodeOfRightChildOfRootPriorToDeletion);
	expectedRightNodeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion) };
	expectedRightNodeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightNodeOfRightChildOfRootPriorToDeletion->parent = expectedRightNodeOfRootPriorToDeletion;
	expectedRightNodeOfRootPriorToDeletion->right = expectedRightNodeOfRightChildOfRootPriorToDeletion;

	// Level 3
	auto expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(6);
	ASSERT_TRUE(expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion);
	expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion) };
	expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftGrandChildOfLeftNodeOfRootPriorToDeletion) };
	expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->left = expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(16);
	ASSERT_TRUE(expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion);
	expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion) };
	expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingRightGrandChildOfLeftNodeOfRootPriorToDeletion) };
	expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion->parent = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftNodeOfLeftChildOfRootPriorToDeletion->right = expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(28);
	ASSERT_TRUE(expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion);
	expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion) };
	expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftGrandChildOfRightNodeOfLeftNodeOfRootPriorToDeletion) };
	expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion->parent = expectedRightNodeOfLeftChildOfRootPriorToDeletion;
	expectedRightNodeOfLeftChildOfRootPriorToDeletion->left = expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion;

	auto expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(56);
	ASSERT_TRUE(expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion);
	expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion.smallestLiteral, idxOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion) };
	expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion.largestLiteral, idxOfClauseFormingLeftGrandChildOfLeftNodeOfRightNodeOfRootPriorToDeletion) };
	expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion->parent = expectedLeftNodeOfRightChildOfRootPriorToDeletion;
	expectedLeftNodeOfRightChildOfRootPriorToDeletion->left = expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion;

	auto expectedLeftmostNodeOfTreePriorToDeletion = std::make_shared<avl::AvlIntervalTreeNode>(2);
	ASSERT_TRUE(expectedLeftmostNodeOfTreePriorToDeletion);
	expectedLeftmostNodeOfTreePriorToDeletion->lowerBoundsSortedAscending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftmostNodeOfTreePriorToDeletion.smallestLiteral, idxOfClauseFormingLeftmostNodeOfTreePriorToDeletion) };
	expectedLeftmostNodeOfTreePriorToDeletion->upperBoundsSortedDescending = { avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftmostNodeOfTreePriorToDeletion.largestLiteral, idxOfClauseFormingLeftmostNodeOfTreePriorToDeletion) };
	expectedLeftmostNodeOfTreePriorToDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedLeftmostNodeOfTreePriorToDeletion->parent = expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion->left = expectedLeftmostNodeOfTreePriorToDeletion;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootPriorToDeletion, avlIntervalTree->getRootNode()));
	ASSERT_EQ(avl::AvlIntervalTreeNode::ClauseRemovalResult::Removed, avlIntervalTree->removeClause(idxOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion, literalBoundsOfClauseFormingRightNodeOfRightChildOfRootPriorToDeletion));

	// Level 0
	avl::AvlIntervalTreeNode::ptr expectedRootAfterDeletion = expectedLeftNodeOfRootPriorToDeletion;
	expectedRootAfterDeletion->parent = nullptr;
	expectedRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	// Level 1
	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfRootAfterDeletion = expectedLeftNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftChildOfRootAfterDeletion->parent = expectedRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedRootAfterDeletion->left = expectedLeftChildOfRootAfterDeletion;

	avl::AvlIntervalTreeNode::ptr expectedRightChildOfRootAfterDeletion = expectedRootPriorToDeletion;
	expectedRightChildOfRootAfterDeletion->parent = expectedRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootAfterDeletion->right = expectedRightChildOfRootAfterDeletion;

	// Level 2
	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfLeftChildOfRootAfterDeletion = expectedLeftGrandChildNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->right = expectedLeftNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;

	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfLeftChildOfRootAfterDeletion = expectedRightGrandChildNodeOfLeftChildOfRootPriorToDeletion;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftChildOfRootAfterDeletion;
	expectedLeftChildOfRootAfterDeletion->right = expectedRightNodeOfLeftChildOfRootAfterDeletion;
	expectedRightNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	avl::AvlIntervalTreeNode::ptr expectedLeftNodeOfRightChildOfRootAfterDeletion = expectedRightNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->left = expectedLeftNodeOfRightChildOfRootAfterDeletion;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	
	avl::AvlIntervalTreeNode::ptr expectedRightNodeOfRightChildOfRootAfterDeletion = expectedLeftNodeOfRightChildOfRootPriorToDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->parent = expectedRightChildOfRootAfterDeletion;
	expectedRightChildOfRootAfterDeletion->right = expectedRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	// Level 3
	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfLeftNodeOfLeftChildOfRootAfterDeletion = expectedLeftmostNodeOfTreePriorToDeletion;
	expectedLeftChildOfLeftNodeOfLeftChildOfRootAfterDeletion->parent = expectedLeftNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftNodeOfLeftChildOfRootAfterDeletion->left = expectedLeftChildOfLeftNodeOfLeftChildOfRootAfterDeletion;
	expectedLeftChildOfLeftNodeOfLeftChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfLeftNodeOfRightChildOfRootAfterDeletion = expectedLeftNodeOfRightNodeOfLeftChildOfRootPriorToDeletion;
	expectedLeftChildOfLeftNodeOfRightChildOfRootAfterDeletion->parent = expectedLeftNodeOfRightChildOfRootAfterDeletion;
	expectedLeftNodeOfRightChildOfRootAfterDeletion->left = expectedLeftChildOfLeftNodeOfRightChildOfRootAfterDeletion;
	expectedLeftChildOfLeftNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	avl::AvlIntervalTreeNode::ptr expectedLeftChildOfRightNodeOfRightChildOfRootAfterDeletion = expectedLeftNodeOfLeftNodeOfRightChildOfRootPriorToDeletion;
	expectedLeftChildOfRightNodeOfRightChildOfRootAfterDeletion->parent = expectedRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->left = expectedLeftChildOfRightNodeOfRightChildOfRootAfterDeletion;
	expectedLeftChildOfRightNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	avl::AvlIntervalTreeNode::ptr expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion = expectedRightNodeOfRootPriorToDeletion;
	expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion->parent = expectedRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightNodeOfRightChildOfRootAfterDeletion->right = expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion;
	expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion->right = nullptr;
	expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion->left = nullptr;
	expectedRightChildOfRightNodeOfRightChildOfRootAfterDeletion->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootAfterDeletion, avlIntervalTree->getRootNode()));
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalNotOverlappingAnyNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr long expectedKeyOfRoot = 0;
	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, 10);
	constexpr std::size_t idxOfSecondClauseInRoot = 1;
	const auto literalBoundsOfSecondClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 5);
	constexpr std::size_t idxOfThirdClauseInRoot = 2;
	const auto literalBoundsOfThirdClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 2);
	constexpr std::size_t idxOfFourthClauseInRoot = 3;
	const auto literalBoundsOFourthClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 7);

	constexpr long expectedKeyOfLeftChildOfRoot = -13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOFRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-15, -10);
	constexpr std::size_t idxOfSecondClauseInLeftChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-17, -13);
	constexpr std::size_t idxOfThirdClauseInLeftChildOfRoot = 6;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-13, -11);

	constexpr long expectedKeyOfRightChildOfRoot = 24;
	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 7;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(20, 28);
	constexpr std::size_t idxOfSecondClauseInRightChildOfRoot = 8;
	const auto literalBoundsOfSecondClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(22, 24);
	constexpr std::size_t idxOfThirdClauseInRightChildOfRoot = 9;
	const auto literalBoundsOfThirdClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(21, 25);

	constexpr long expectedKeyOfRightChildOfLeftChildOfRoot = -9;
	constexpr std::size_t idxOfClauseFormingRightChildOfLeftChildOfRoot = 10;
	const auto literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -8);
	constexpr std::size_t idxOfSecondClauseInRightChildOfLeftChildOfRoot = 11;
	const auto literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -9);

	constexpr long expectedKeyOfLeftChildOfRightChildOfRoot = 13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRightChildOfRoot = 12;
	const auto literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);
	constexpr std::size_t idxOfSecondClauseFormingLeftChildOfRightChildOfRoot = 13;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(11, 16);
	constexpr std::size_t idxOfThirdClauseFormingLeftChildOfRightChildOfRoot = 14;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 13);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRoot, literalBoundsOfSecondClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRoot, literalBoundsOfThirdClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFourthClauseInRoot, literalBoundsOFourthClauseInRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOFRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInLeftChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInLeftChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfRoot, literalBoundsOfSecondClauseInRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRightChildOfRoot, literalBoundsOfThirdClauseInRightChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfLeftChildOfRoot, literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRoot);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.smallestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.smallestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.smallestLiteral, idxOfThirdClauseInRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.largestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.largestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.largestLiteral, idxOfThirdClauseInRoot)
	};

	auto expectedNodeOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRoot);
	expectedNodeOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedNodeOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.smallestLiteral, idxOfThirdClauseInLeftChildOfRoot)
	};
	expectedNodeOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.largestLiteral, idxOfThirdClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.largestLiteral, idxOfSecondClauseInLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfLeftChildOfRoot);
	expectedNodeOfRightChildOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfRightChildOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};
	expectedNodeOfRightChildOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfRoot);
	expectedNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNodeOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.smallestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};
	expectedNodeOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.largestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};


	auto expectedNodeOfLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRightChildOfRoot);
	expectedNodeOfLeftChildOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfLeftChildOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot)
	};
	expectedNodeOfLeftChildOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot)
	};

	expectedRootNode->left = expectedNodeOfLeftChildOfRoot;
	expectedNodeOfLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedNodeOfRightChildOfRoot;
	expectedNodeOfRightChildOfRoot->parent = expectedRootNode;

	expectedNodeOfLeftChildOfRoot->right = expectedNodeOfRightChildOfLeftChildOfRoot;
	expectedNodeOfRightChildOfLeftChildOfRoot->parent = expectedNodeOfLeftChildOfRoot;

	expectedNodeOfRightChildOfRoot->left = expectedNodeOfLeftChildOfRightChildOfRoot;
	expectedNodeOfLeftChildOfRightChildOfRoot->parent = expectedNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));


	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralSmallerThanAnyFoundInWholeTree = avlIntervalTree->getOverlappingIntervalsForLiteral(-200);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClauseIndicesForLiteralSmallerThanAnyFoundInWholeTree));

	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralNotFoundInAnyNodeInWholeTree = avlIntervalTree->getOverlappingIntervalsForLiteral(17);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClauseIndicesForLiteralNotFoundInAnyNodeInWholeTree));

	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralLargerThanAnyFoundInWholeTree = avlIntervalTree->getOverlappingIntervalsForLiteral(200);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClauseIndicesForLiteralLargerThanAnyFoundInWholeTree));
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingOnlyOneNode)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr long expectedKeyOfRoot = 1;
	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, 10);
	constexpr std::size_t idxOfSecondClauseInRoot = 1;
	const auto literalBoundsOfSecondClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 5);
	constexpr std::size_t idxOfThirdClauseInRoot = 2;
	const auto literalBoundsOfThirdClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 2);
	constexpr std::size_t idxOfFourthClauseInRoot = 3;
	const auto literalBoundsOFourthClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 7);

	constexpr long expectedKeyOfLeftChildOfRoot = -13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOFRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-15, -11);
	constexpr std::size_t idxOfSecondClauseInLeftChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-17, -13);
	constexpr std::size_t idxOfThirdClauseInLeftChildOfRoot = 6;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-13, -11);

	constexpr long expectedKeyOfRightChildOfRoot = 24;
	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 7;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(20, 28);
	constexpr std::size_t idxOfSecondClauseInRightChildOfRoot = 8;
	const auto literalBoundsOfSecondClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(22, 24);
	constexpr std::size_t idxOfThirdClauseInRightChildOfRoot = 9;
	const auto literalBoundsOfThirdClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(21, 25);

	constexpr long expectedKeyOfRightChildOfLeftChildOfRoot = -9;
	constexpr std::size_t idxOfClauseFormingRightChildOfLeftChildOfRoot = 10;
	const auto literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -8);
	constexpr std::size_t idxOfSecondClauseInRightChildOfLeftChildOfRoot = 11;
	const auto literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -9);

	constexpr long expectedKeyOfLeftChildOfRightChildOfRoot = 13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRightChildOfRoot = 12;
	const auto literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);
	constexpr std::size_t idxOfSecondClauseFormingLeftChildOfRightChildOfRoot = 13;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(11, 16);
	constexpr std::size_t idxOfThirdClauseFormingLeftChildOfRightChildOfRoot = 14;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 13);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRoot, literalBoundsOfSecondClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRoot, literalBoundsOfThirdClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFourthClauseInRoot, literalBoundsOFourthClauseInRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOFRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInLeftChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInLeftChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfRoot, literalBoundsOfSecondClauseInRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRightChildOfRoot, literalBoundsOfThirdClauseInRightChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfLeftChildOfRoot, literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRoot);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.smallestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.smallestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.smallestLiteral, idxOfThirdClauseInRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.largestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.largestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.largestLiteral, idxOfThirdClauseInRoot)
	};

	auto expectedNodeOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRoot);
	expectedNodeOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedNodeOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.smallestLiteral, idxOfThirdClauseInLeftChildOfRoot)
	};
	expectedNodeOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.largestLiteral, idxOfThirdClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.largestLiteral, idxOfSecondClauseInLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfLeftChildOfRoot);
	expectedNodeOfRightChildOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfRightChildOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};
	expectedNodeOfRightChildOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfRoot);
	expectedNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNodeOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.smallestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};
	expectedNodeOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.largestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};


	auto expectedNodeOfLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRightChildOfRoot);
	expectedNodeOfLeftChildOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfLeftChildOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot)
	};
	expectedNodeOfLeftChildOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot)
	};

	expectedRootNode->left = expectedNodeOfLeftChildOfRoot;
	expectedNodeOfLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedNodeOfRightChildOfRoot;
	expectedNodeOfRightChildOfRoot->parent = expectedRootNode;

	expectedNodeOfLeftChildOfRoot->right = expectedNodeOfRightChildOfLeftChildOfRoot;
	expectedNodeOfRightChildOfLeftChildOfRoot->parent = expectedNodeOfLeftChildOfRoot;

	expectedNodeOfRightChildOfRoot->left = expectedNodeOfLeftChildOfRightChildOfRoot;
	expectedNodeOfLeftChildOfRightChildOfRoot->parent = expectedNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralOnlyFoundInRightChildOfLeftChildOfRoot = { idxOfClauseFormingRightChildOfLeftChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralOnlyFoundInRightChildOfLeftChildOfRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(-10);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralOnlyFoundInRightChildOfLeftChildOfRoot, actualOverlappingClauseIndicesForLiteralOnlyFoundInRightChildOfLeftChildOfRoot));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralOnlyFoundInLeftChildOfRightChildOfRoot = { idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralOnlyFoundInLeftChildOfRightChildOfRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(11);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralOnlyFoundInLeftChildOfRightChildOfRoot, actualOverlappingClauseIndicesForLiteralOnlyFoundInLeftChildOfRightChildOfRoot));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralOnlyFoundInRoot = { idxOfClauseFormingRoot, idxOfFourthClauseInRoot, idxOfSecondClauseInRoot, idxOfThirdClauseInRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralOnlyFoundInRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(2);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralOnlyFoundInRoot, actualOverlappingClauseIndicesForLiteralOnlyFoundInRoot));

}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingNodeAndOneOfItsRightChildren)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr long expectedKeyOfRoot = 3;
	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, 15);
	constexpr std::size_t idxOfSecondClauseInRoot = 1;
	const auto literalBoundsOfSecondClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 5);
	constexpr std::size_t idxOfThirdClauseInRoot = 2;
	const auto literalBoundsOfThirdClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 3);
	constexpr std::size_t idxOfFourthClauseInRoot = 3;
	const auto literalBoundsOFourthClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 7);

	constexpr long expectedKeyOfLeftChildOfRoot = -13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOFRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-15, -11);
	constexpr std::size_t idxOfSecondClauseInLeftChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-17, -13);
	constexpr std::size_t idxOfThirdClauseInLeftChildOfRoot = 6;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-13, -11);

	constexpr long expectedKeyOfRightChildOfRoot = 24;
	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 7;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(20, 28);
	constexpr std::size_t idxOfSecondClauseInRightChildOfRoot = 8;
	const auto literalBoundsOfSecondClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(22, 24);
	constexpr std::size_t idxOfThirdClauseInRightChildOfRoot = 9;
	const auto literalBoundsOfThirdClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(21, 25);

	constexpr long expectedKeyOfRightChildOfLeftChildOfRoot = -9;
	constexpr std::size_t idxOfClauseFormingRightChildOfLeftChildOfRoot = 10;
	const auto literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -8);
	constexpr std::size_t idxOfSecondClauseInRightChildOfLeftChildOfRoot = 11;
	const auto literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -9);

	constexpr long expectedKeyOfLeftChildOfRightChildOfRoot = 13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRightChildOfRoot = 12;
	const auto literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);
	constexpr std::size_t idxOfSecondClauseFormingLeftChildOfRightChildOfRoot = 13;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(11, 16);
	constexpr std::size_t idxOfThirdClauseFormingLeftChildOfRightChildOfRoot = 14;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 13);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRoot, literalBoundsOfSecondClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRoot, literalBoundsOfThirdClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFourthClauseInRoot, literalBoundsOFourthClauseInRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOFRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInLeftChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInLeftChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfRoot, literalBoundsOfSecondClauseInRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRightChildOfRoot, literalBoundsOfThirdClauseInRightChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfLeftChildOfRoot, literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRoot);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.smallestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.smallestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.smallestLiteral, idxOfThirdClauseInRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.largestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.largestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.largestLiteral, idxOfThirdClauseInRoot)
	};

	auto expectedNodeOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRoot);
	expectedNodeOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedNodeOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.smallestLiteral, idxOfThirdClauseInLeftChildOfRoot)
	};
	expectedNodeOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.largestLiteral, idxOfThirdClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.largestLiteral, idxOfSecondClauseInLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfLeftChildOfRoot);
	expectedNodeOfRightChildOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfRightChildOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};
	expectedNodeOfRightChildOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfRoot);
	expectedNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNodeOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.smallestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};
	expectedNodeOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.largestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};


	auto expectedNodeOfLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRightChildOfRoot);
	expectedNodeOfLeftChildOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfLeftChildOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot)
	};
	expectedNodeOfLeftChildOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot)
	};

	expectedRootNode->left = expectedNodeOfLeftChildOfRoot;
	expectedNodeOfLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedNodeOfRightChildOfRoot;
	expectedNodeOfRightChildOfRoot->parent = expectedRootNode;

	expectedNodeOfLeftChildOfRoot->right = expectedNodeOfRightChildOfLeftChildOfRoot;
	expectedNodeOfRightChildOfLeftChildOfRoot->parent = expectedNodeOfLeftChildOfRoot;

	expectedNodeOfRightChildOfRoot->left = expectedNodeOfLeftChildOfRightChildOfRoot;
	expectedNodeOfLeftChildOfRightChildOfRoot->parent = expectedNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralOnlyInRootAndLeftChildOfRightChildOfRoot = { idxOfClauseFormingRoot, idxOfClauseFormingLeftChildOfRightChildOfRoot, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralOnlyInRootAndLeftChildOfRightChildOfRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(14);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralOnlyInRootAndLeftChildOfRightChildOfRoot, actualOverlappingClauseIndicesForLiteralOnlyInRootAndLeftChildOfRightChildOfRoot));
}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalOverlappingNodeAndOneOfItsLeftChildren)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr long expectedKeyOfRoot = 1;
	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, 10);
	constexpr std::size_t idxOfSecondClauseInRoot = 1;
	const auto literalBoundsOfSecondClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 5);
	constexpr std::size_t idxOfThirdClauseInRoot = 2;
	const auto literalBoundsOfThirdClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 2);
	constexpr std::size_t idxOfFourthClauseInRoot = 3;
	const auto literalBoundsOFourthClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 7);

	constexpr long expectedKeyOfLeftChildOfRoot = -13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOFRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-15, -11);
	constexpr std::size_t idxOfSecondClauseInLeftChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-17, -13);
	constexpr std::size_t idxOfThirdClauseInLeftChildOfRoot = 6;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-13, -11);

	constexpr long expectedKeyOfRightChildOfRoot = 24;
	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 7;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(20, 28);
	constexpr std::size_t idxOfSecondClauseInRightChildOfRoot = 8;
	const auto literalBoundsOfSecondClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(22, 24);
	constexpr std::size_t idxOfThirdClauseInRightChildOfRoot = 9;
	const auto literalBoundsOfThirdClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(21, 25);

	constexpr long expectedKeyOfRightChildOfLeftChildOfRoot = -9;
	constexpr std::size_t idxOfClauseFormingRightChildOfLeftChildOfRoot = 10;
	const auto literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -8);
	constexpr std::size_t idxOfSecondClauseInRightChildOfLeftChildOfRoot = 11;
	const auto literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -9);

	constexpr long expectedKeyOfLeftChildOfRightChildOfRoot = 13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRightChildOfRoot = 12;
	const auto literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);
	constexpr std::size_t idxOfSecondClauseFormingLeftChildOfRightChildOfRoot = 13;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(11, 16);
	constexpr std::size_t idxOfThirdClauseFormingLeftChildOfRightChildOfRoot = 14;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 13);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRoot, literalBoundsOfSecondClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRoot, literalBoundsOfThirdClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFourthClauseInRoot, literalBoundsOFourthClauseInRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOFRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInLeftChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInLeftChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfRoot, literalBoundsOfSecondClauseInRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRightChildOfRoot, literalBoundsOfThirdClauseInRightChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfLeftChildOfRoot, literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRoot);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.smallestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.smallestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.smallestLiteral, idxOfThirdClauseInRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.largestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.largestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.largestLiteral, idxOfThirdClauseInRoot)
	};

	auto expectedNodeOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRoot);
	expectedNodeOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedNodeOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.smallestLiteral, idxOfThirdClauseInLeftChildOfRoot)
	};
	expectedNodeOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.largestLiteral, idxOfThirdClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.largestLiteral, idxOfSecondClauseInLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfLeftChildOfRoot);
	expectedNodeOfRightChildOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfRightChildOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};
	expectedNodeOfRightChildOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfRoot);
	expectedNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNodeOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.smallestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};
	expectedNodeOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.largestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};


	auto expectedNodeOfLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRightChildOfRoot);
	expectedNodeOfLeftChildOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfLeftChildOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot)
	};
	expectedNodeOfLeftChildOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot)
	};

	expectedRootNode->left = expectedNodeOfLeftChildOfRoot;
	expectedNodeOfLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedNodeOfRightChildOfRoot;
	expectedNodeOfRightChildOfRoot->parent = expectedRootNode;

	expectedNodeOfLeftChildOfRoot->right = expectedNodeOfRightChildOfLeftChildOfRoot;
	expectedNodeOfRightChildOfLeftChildOfRoot->parent = expectedNodeOfLeftChildOfRoot;

	expectedNodeOfRightChildOfRoot->left = expectedNodeOfLeftChildOfRightChildOfRoot;
	expectedNodeOfLeftChildOfRightChildOfRoot->parent = expectedNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralFoundInRootAndRightChildOfLeftChildOfRoot = { idxOfClauseFormingRoot, idxOfClauseFormingRightChildOfLeftChildOfRoot, idxOfSecondClauseInRightChildOfLeftChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralFoundInRootAndRightChildOfLeftChildOfRoot = avlIntervalTree->getOverlappingIntervalsForLiteral(-9);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralFoundInRootAndRightChildOfLeftChildOfRoot, actualOverlappingClauseIndicesForLiteralFoundInRootAndRightChildOfLeftChildOfRoot));

}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryInEmptyTree)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);
	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(nullptr, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralsNotFoundInWholeTree = avlIntervalTree->getOverlappingIntervalsForLiteral(-9);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(EMPTY_CLAUSE_INDICES_RESULT, actualOverlappingClauseIndicesForLiteralsNotFoundInWholeTree));

}

TEST_F(AvlIntervalTreeTestsFixture, PerformStabbingQueryForIntervalMatchingMultipleNodeConnectedViaMultipleLevels)
{
	auto avlIntervalTree = std::make_unique<avl::OpaqueAvlIntervalTree>();
	ASSERT_TRUE(avlIntervalTree);

	constexpr long expectedKeyOfRoot = 0;
	constexpr std::size_t idxOfClauseFormingRoot = 0;
	const auto literalBoundsOfClauseFormingRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, 10);
	constexpr std::size_t idxOfSecondClauseInRoot = 1;
	const auto literalBoundsOfSecondClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 5);
	constexpr std::size_t idxOfThirdClauseInRoot = 2;
	const auto literalBoundsOfThirdClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-3, 2);
	constexpr std::size_t idxOfFourthClauseInRoot = 3;
	const auto literalBoundsOFourthClauseInRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-8, 7);

	constexpr long expectedKeyOfLeftChildOfRoot = -13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRoot = 4;
	const auto literalBoundsOfClauseFormingLeftChildOFRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-15, -10);
	constexpr std::size_t idxOfSecondClauseInLeftChildOfRoot = 5;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-17, -13);
	constexpr std::size_t idxOfThirdClauseInLeftChildOfRoot = 6;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-13, -11);

	constexpr long expectedKeyOfRightChildOfRoot = 24;
	constexpr std::size_t idxOfClauseFormingRightChildOfRoot = 7;
	const auto literalBoundsOfClauseFormingRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(20, 28);
	constexpr std::size_t idxOfSecondClauseInRightChildOfRoot = 8;
	const auto literalBoundsOfSecondClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(22, 24);
	constexpr std::size_t idxOfThirdClauseInRightChildOfRoot = 9;
	const auto literalBoundsOfThirdClauseInRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(21, 25);

	constexpr long expectedKeyOfRightChildOfLeftChildOfRoot = -9;
	constexpr std::size_t idxOfClauseFormingRightChildOfLeftChildOfRoot = 10;
	const auto literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-10, -8);
	constexpr std::size_t idxOfSecondClauseInRightChildOfLeftChildOfRoot = 11;
	const auto literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(-9, -9);

	constexpr long expectedKeyOfLeftChildOfRightChildOfRoot = 13;
	constexpr std::size_t idxOfClauseFormingLeftChildOfRightChildOfRoot = 12;
	const auto literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(12, 14);
	constexpr std::size_t idxOfSecondClauseFormingLeftChildOfRightChildOfRoot = 13;
	const auto literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(11, 16);
	constexpr std::size_t idxOfThirdClauseFormingLeftChildOfRightChildOfRoot = 14;
	const auto literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot = dimacs::ProblemDefinition::Clause::LiteralBounds(10, 13);

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRoot, literalBoundsOfClauseFormingRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRoot, literalBoundsOfSecondClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRoot, literalBoundsOfThirdClauseInRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfFourthClauseInRoot, literalBoundsOFourthClauseInRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRoot, literalBoundsOfClauseFormingLeftChildOFRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInLeftChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInLeftChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfRoot, literalBoundsOfClauseFormingRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfRoot, literalBoundsOfSecondClauseInRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseInRightChildOfRoot, literalBoundsOfThirdClauseInRightChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingRightChildOfLeftChildOfRoot, literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseInRightChildOfLeftChildOfRoot, literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot));

	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfSecondClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot));
	ASSERT_TRUE(avlIntervalTree->insertClause(idxOfThirdClauseFormingLeftChildOfRightChildOfRoot, literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot));


	auto expectedRootNode = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRoot);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedRootNode->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.smallestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.smallestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.smallestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.smallestLiteral, idxOfThirdClauseInRoot)
	};
	expectedRootNode->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRoot.largestLiteral, idxOfClauseFormingRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOFourthClauseInRoot.largestLiteral, idxOfFourthClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRoot.largestLiteral, idxOfSecondClauseInRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRoot.largestLiteral, idxOfThirdClauseInRoot)
	};

	auto expectedNodeOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRoot);
	expectedNodeOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
	expectedNodeOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.smallestLiteral, idxOfThirdClauseInLeftChildOfRoot)
	};
	expectedNodeOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOFRoot.largestLiteral, idxOfClauseFormingLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRoot.largestLiteral, idxOfThirdClauseInLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRoot.largestLiteral, idxOfSecondClauseInLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfLeftChildOfRoot);
	expectedNodeOfRightChildOfLeftChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfRightChildOfLeftChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};
	expectedNodeOfRightChildOfLeftChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfLeftChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfLeftChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfLeftChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfLeftChildOfRoot)
	};

	auto expectedNodeOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfRightChildOfRoot);
	expectedNodeOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	expectedNodeOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.smallestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.smallestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.smallestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};
	expectedNodeOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingRightChildOfRoot.largestLiteral, idxOfClauseFormingRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseInRightChildOfRoot.largestLiteral, idxOfThirdClauseInRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseInRightChildOfRoot.largestLiteral, idxOfSecondClauseInRightChildOfRoot)
	};


	auto expectedNodeOfLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlIntervalTreeNode>(expectedKeyOfLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedNodeOfLeftChildOfRightChildOfRoot);
	expectedNodeOfLeftChildOfRightChildOfRoot->internalAvlBalancingFactor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
	expectedNodeOfLeftChildOfRightChildOfRoot->lowerBoundsSortedAscending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.smallestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot)
	};
	expectedNodeOfLeftChildOfRightChildOfRoot->upperBoundsSortedDescending = {
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfSecondClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfSecondClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfClauseFormingLeftChildOfRightChildOfRoot),
		avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBoundsOfThirdClauseFormingLeftChildOfRightChildOfRoot.largestLiteral, idxOfThirdClauseFormingLeftChildOfRightChildOfRoot)
	};

	expectedRootNode->left = expectedNodeOfLeftChildOfRoot;
	expectedNodeOfLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedNodeOfRightChildOfRoot;
	expectedNodeOfRightChildOfRoot->parent = expectedRootNode;

	expectedNodeOfLeftChildOfRoot->right = expectedNodeOfRightChildOfLeftChildOfRoot;
	expectedNodeOfRightChildOfLeftChildOfRoot->parent = expectedNodeOfLeftChildOfRoot;

	expectedNodeOfRightChildOfRoot->left = expectedNodeOfLeftChildOfRightChildOfRoot;
	expectedNodeOfLeftChildOfRightChildOfRoot->parent = expectedNodeOfRightChildOfRoot;

	ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expectedRootNode, avlIntervalTree->getRootNode()));

	const std::vector<std::size_t> expectedOverlappingClauseIndicesForLiteralFoundInCompleteLeftSideOfTree = { idxOfClauseFormingRoot, idxOfClauseFormingLeftChildOfRoot, idxOfClauseFormingRightChildOfLeftChildOfRoot };
	const std::vector<std::size_t> actualOverlappingClauseIndicesForLiteralFoundInCompleteLeftSideOfTree = avlIntervalTree->getOverlappingIntervalsForLiteral(-10);
	ASSERT_NO_FATAL_FAILURE(assertThatClauseIndicesOfOverlappingIntervalsMatch(expectedOverlappingClauseIndicesForLiteralFoundInCompleteLeftSideOfTree, actualOverlappingClauseIndicesForLiteralFoundInCompleteLeftSideOfTree));

}
