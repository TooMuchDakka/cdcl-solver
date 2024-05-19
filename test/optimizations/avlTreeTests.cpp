#include "avlTreeTestsFixture.hpp"
#include "problemDefinition.hpp"

TEST_F(AvlTreeTestsFixture, InsertIntoEmptyAvlTree)
{
	const std::vector<long> clauseLiterals = { 1, -2, 3 };
	const auto& firstClause = std::make_shared<dimacs::ProblemDefinition::Clause>(clauseLiterals);
	ASSERT_TRUE(firstClause);
	firstClause->sortLiterals();
	
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, clauseLiterals));

	const avl::AvlTree::AvlTreeNode::constPtr expectedRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(1);
	ASSERT_TRUE(expectedRootNode);

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(-2);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedLeftChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(3);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRightChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;

	expectedRootNode->left = expectedLeftChildOfRoot;
	expectedRootNode->right = expectedRightChildOfRoot;
	expectedRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	
	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expectedRootNode, actualRootNode));
	
	EXPECT_FALSE(avlTreeInstance->find(-1));
	EXPECT_FALSE(avlTreeInstance->find(2));
	EXPECT_FALSE(avlTreeInstance->find(-3));
	EXPECT_FALSE(avlTreeInstance->find(4));
}

TEST_F(AvlTreeTestsFixture, RotateLeft)
{
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);

	const std::vector<long> literalsToInsert = { 1, 1, -2, -3, -3 };
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, literalsToInsert));

	const avl::AvlTree::AvlTreeNode::constPtr expectedRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(-2);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(-3);
	ASSERT_TRUE(expectedLeftChildOfRootNode);
	expectedLeftChildOfRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfRootNode->parent = expectedRootNode;
	ASSERT_TRUE(expectedLeftChildOfRootNode->incrementReferenceCount());

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(1);
	ASSERT_TRUE(expectedRightChildOfRootNode);
	expectedRightChildOfRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfRootNode->parent = expectedRootNode;
	ASSERT_TRUE(expectedRightChildOfRootNode->incrementReferenceCount());

	expectedRootNode->left = expectedLeftChildOfRootNode;
	expectedRootNode->right = expectedRightChildOfRootNode;

	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expectedRootNode, actualRootNode));
	
	EXPECT_TRUE(avlTreeInstance->find(1));
	EXPECT_TRUE(avlTreeInstance->find(-2));
	EXPECT_TRUE(avlTreeInstance->find(-3));

	EXPECT_FALSE(avlTreeInstance->find(-1));
	EXPECT_FALSE(avlTreeInstance->find(2));
	EXPECT_FALSE(avlTreeInstance->find(3));
}

TEST_F(AvlTreeTestsFixture, RotateRight)
{
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);

	const std::vector<long> literalsToInsert = { 1, 1, 2, 3, 3 };
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, literalsToInsert));

	const avl::AvlTree::AvlTreeNode::constPtr expectedRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(2);
	ASSERT_TRUE(expectedRootNode);
	expectedRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(1);
	ASSERT_TRUE(expectedLeftChildOfRootNode);
	expectedLeftChildOfRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfRootNode->parent = expectedRootNode;
	ASSERT_TRUE(expectedLeftChildOfRootNode->incrementReferenceCount());

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(3);
	ASSERT_TRUE(expectedRightChildOfRootNode);
	expectedRightChildOfRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfRootNode->parent = expectedRootNode;
	ASSERT_TRUE(expectedRightChildOfRootNode->incrementReferenceCount());

	expectedRootNode->left = expectedLeftChildOfRootNode;
	expectedRootNode->right = expectedRightChildOfRootNode;

	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expectedRootNode, actualRootNode));

	EXPECT_TRUE(avlTreeInstance->find(1));
	EXPECT_TRUE(avlTreeInstance->find(2));
	EXPECT_TRUE(avlTreeInstance->find(3));

	EXPECT_FALSE(avlTreeInstance->find(-1));
	EXPECT_FALSE(avlTreeInstance->find(-2));
	EXPECT_FALSE(avlTreeInstance->find(-3));
}

TEST_F(AvlTreeTestsFixture, RotateLeftRight)
{
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);

	const std::vector<long> literalsToInsert = { 3, -1, 3, 4, -3, -2, -3 };
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, literalsToInsert));

	const avl::AvlTree::AvlTreeNode::constPtr expectedRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(3);
	ASSERT_TRUE(expectedRootNode);
	ASSERT_TRUE(expectedRootNode->incrementReferenceCount());
	expectedRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::LEFT_HEAVY;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(-2);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(4);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfLeftChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(-3);
	ASSERT_TRUE(expectedLeftChildOfLeftChildOfRoot);
	ASSERT_TRUE(expectedLeftChildOfLeftChildOfRoot->incrementReferenceCount());
	expectedLeftChildOfLeftChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfLeftChildOfRoot->parent = expectedLeftChildOfRoot;
	expectedLeftChildOfRoot->left = expectedLeftChildOfLeftChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfLeftChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(-1);
	ASSERT_TRUE(expectedRightChildOfLeftChildOfRoot);
	expectedRightChildOfLeftChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfLeftChildOfRoot->parent = expectedLeftChildOfRoot;
	expectedLeftChildOfRoot->right = expectedRightChildOfLeftChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expectedRootNode, actualRootNode));

	EXPECT_TRUE(avlTreeInstance->find(-3));
	EXPECT_TRUE(avlTreeInstance->find(-2));
	EXPECT_TRUE(avlTreeInstance->find(-1));
	EXPECT_TRUE(avlTreeInstance->find(3));
	EXPECT_TRUE(avlTreeInstance->find(4));

	EXPECT_FALSE(avlTreeInstance->find(0));
	EXPECT_FALSE(avlTreeInstance->find(1));
	EXPECT_FALSE(avlTreeInstance->find(2));
	EXPECT_FALSE(avlTreeInstance->find(-4));
}

TEST_F(AvlTreeTestsFixture, RotateRightLeft)
{
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);

	const std::vector<long> literalsToInsert = { 3, -1, 3, 6, 5, 7, 5 };
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, literalsToInsert));

	const avl::AvlTree::AvlTreeNode::constPtr expectedRootNode = std::make_shared<avl::AvlTree::AvlTreeNode>(3);
	ASSERT_TRUE(expectedRootNode);
	ASSERT_TRUE(expectedRootNode->incrementReferenceCount());
	expectedRootNode->balancingFactor = avl::AvlTree::AvlTreeNode::RIGHT_HEAVY;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(-1);
	ASSERT_TRUE(expectedLeftChildOfRoot);
	expectedLeftChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfRoot->parent = expectedRootNode;
	expectedRootNode->left = expectedLeftChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(6);
	ASSERT_TRUE(expectedRightChildOfRoot);
	expectedRightChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfRoot->parent = expectedRootNode;
	expectedRootNode->right = expectedRightChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedLeftChildOfRightChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(5);
	ASSERT_TRUE(expectedLeftChildOfRightChildOfRoot);
	ASSERT_TRUE(expectedLeftChildOfRightChildOfRoot->incrementReferenceCount());
	expectedLeftChildOfRightChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedLeftChildOfRightChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->left = expectedLeftChildOfRightChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr expectedRightChildOfRightChildOfRoot = std::make_shared<avl::AvlTree::AvlTreeNode>(7);
	ASSERT_TRUE(expectedRightChildOfRightChildOfRoot);
	expectedRightChildOfRightChildOfRoot->balancingFactor = avl::AvlTree::AvlTreeNode::BALANCED;
	expectedRightChildOfRightChildOfRoot->parent = expectedRightChildOfRoot;
	expectedRightChildOfRoot->right = expectedRightChildOfRightChildOfRoot;

	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expectedRootNode, actualRootNode));

	EXPECT_TRUE(avlTreeInstance->find(-1));
	EXPECT_TRUE(avlTreeInstance->find(3));
	EXPECT_TRUE(avlTreeInstance->find(5));
	EXPECT_TRUE(avlTreeInstance->find(6));
	EXPECT_TRUE(avlTreeInstance->find(7));

	EXPECT_FALSE(avlTreeInstance->find(-7));
	EXPECT_FALSE(avlTreeInstance->find(-6));
	EXPECT_FALSE(avlTreeInstance->find(-5));
	EXPECT_FALSE(avlTreeInstance->find(-3));
	EXPECT_FALSE(avlTreeInstance->find(1));
}

TEST_F(AvlTreeTestsFixture, MultipleRotations)
{
	const auto avlTreeInstance = std::make_unique<avl::OpaqueAvlTree>();
	ASSERT_TRUE(avlTreeInstance);

	// Literals to insert: m, n, o, l, k , q, p , h , i, a
	constexpr long mNodeKey = 'm';
	constexpr long nNodeKey = 'n';
	constexpr long oNodeKey = 'o';
	constexpr long lNodeKey = 'l';
	constexpr long kNodeKey = 'k';
	constexpr long qNodeKey = 'q';
	constexpr long pNodeKey = 'p';
	constexpr long hNodeKey = 'h';
	constexpr long iNodeKey = 'i';
	constexpr long aNodeKey = 'a';

	const std::vector<long> literalsToInsert = {mNodeKey, nNodeKey, oNodeKey, lNodeKey, kNodeKey, qNodeKey, pNodeKey, hNodeKey, iNodeKey, aNodeKey};

	/*
	 * Expected tree:
	 *
	 *					n	
	 *		i							p
	 *	h			l				o		q
	 * a		k		m
	 */
	ASSERT_NO_FATAL_FAILURE(assertThatInsertOfLiteralsDoesNotFail(*avlTreeInstance, literalsToInsert));

	const avl::AvlTree::AvlTreeNode::constPtr nNode = std::make_shared<avl::AvlTree::AvlTreeNode>(nNodeKey);
	EXPECT_TRUE(nNode);
	nNode->balancingFactor = avl::AvlTree::AvlTreeNode::LEFT_HEAVY;

	const avl::AvlTree::AvlTreeNode::constPtr iNode = std::make_shared<avl::AvlTree::AvlTreeNode>(iNodeKey);
	EXPECT_TRUE(iNode);
	iNode->parent = nNode;
	nNode->left = iNode;

	const avl::AvlTree::AvlTreeNode::constPtr hNode = std::make_shared<avl::AvlTree::AvlTreeNode>(hNodeKey);
	EXPECT_TRUE(hNode);
	hNode->balancingFactor = avl::AvlTree::AvlTreeNode::LEFT_HEAVY;
	hNode->parent = iNode;
	iNode->left = hNode;

	const avl::AvlTree::AvlTreeNode::constPtr aNode = std::make_shared<avl::AvlTree::AvlTreeNode>(aNodeKey);
	EXPECT_TRUE(aNode);
	aNode->parent = hNode;
	hNode->left = aNode;

	const avl::AvlTree::AvlTreeNode::constPtr lNode = std::make_shared<avl::AvlTree::AvlTreeNode>(lNodeKey);
	EXPECT_TRUE(lNode);
	lNode->parent = iNode;
	iNode->right = lNode;

	const avl::AvlTree::AvlTreeNode::constPtr kNode = std::make_shared<avl::AvlTree::AvlTreeNode>(kNodeKey);
	EXPECT_TRUE(kNode);
	kNode->parent = lNode;
	lNode->left = kNode;

	const avl::AvlTree::AvlTreeNode::constPtr mNode = std::make_shared<avl::AvlTree::AvlTreeNode>(mNodeKey);
	EXPECT_TRUE(mNode);
	mNode->parent = lNode;
	lNode->right = mNode;

	const avl::AvlTree::AvlTreeNode::constPtr pNode = std::make_shared<avl::AvlTree::AvlTreeNode>(pNodeKey);
	EXPECT_TRUE(pNode);
	pNode->parent = nNode;
	nNode->right = pNode;

	const avl::AvlTree::AvlTreeNode::constPtr oNode = std::make_shared<avl::AvlTree::AvlTreeNode>(oNodeKey);
	EXPECT_TRUE(oNode);
	oNode->parent = pNode;
	pNode->left = oNode;

	const avl::AvlTree::AvlTreeNode::constPtr qNode = std::make_shared<avl::AvlTree::AvlTreeNode>(qNodeKey);
	qNode->parent = pNode;
	pNode->right = qNode;
	EXPECT_TRUE(qNode);

	const avl::AvlTree::AvlTreeNode::constPtr actualRootNode = avlTreeInstance->getRootNode();
	ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(nNode, actualRootNode));

	EXPECT_TRUE(avlTreeInstance->find(nNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(iNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(hNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(aNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(lNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(kNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(mNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(pNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(oNodeKey));
	EXPECT_TRUE(avlTreeInstance->find(qNodeKey));
}