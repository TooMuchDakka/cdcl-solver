#include "optimizations/blockedClauseElimination/avlIntervalTree.hpp"

#include <ios>

using namespace avl;

// TODO: Insert does not check whether the clause was already inserted into the tree

bool AvlIntervalTree::insertClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (literalBounds.smallestLiteral > literalBounds.largestLiteral || !literalBounds.smallestLiteral || !literalBounds.largestLiteral)
		return false;

	AvlIntervalTreeNode::ptr newTreeNode = std::make_shared<AvlIntervalTreeNode>(AvlIntervalTreeNode::determineLiteralBoundsMidPoint(literalBounds));
	newTreeNode->insertLowerBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.smallestLiteral, clauseIdxInFormula));
	newTreeNode->insertUpperBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.largestLiteral, clauseIdxInFormula));

	if (!newTreeNode)
		return false;

	if (!root)
	{
		root = newTreeNode;
		return true;
	}

	AvlIntervalTreeNode::ptr traversalNode = root;
	while (traversalNode)
	{
		if (traversalNode->doesClauseIntersect(literalBounds) && traversalNode->isKeyContainedInInterval(literalBounds))
		{
			traversalNode->insertLowerBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.smallestLiteral, clauseIdxInFormula));
			traversalNode->insertUpperBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.largestLiteral, clauseIdxInFormula));
			return true;
		}

		if (literalBounds.largestLiteral < traversalNode->key)
		{
			if (traversalNode->left)
			{
				traversalNode = traversalNode->left;
			}
			else
			{
				traversalNode->left = newTreeNode;
				newTreeNode->parent = traversalNode;
				traversalNode = nullptr;
			}
		}
		else
		{
			if (traversalNode->right)
			{
				traversalNode = traversalNode->right;
			}
			else
			{
				traversalNode->right = newTreeNode;
				newTreeNode->parent = traversalNode;
				traversalNode = nullptr;
			}
		}
	}

	traversalNode = newTreeNode;
	for (AvlIntervalTreeNode::ptr parentNode = traversalNode->parent; parentNode; parentNode = traversalNode->parent)
	{
		AvlIntervalTreeNode::ptr grandParentNode = parentNode->parent;
		AvlIntervalTreeNode::ptr subTreeRootAfterRotation = nullptr;
		if (traversalNode == parentNode->left)
		{
			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
			{
				if (traversalNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
					subTreeRootAfterRotation = rotateLeftRight(parentNode, traversalNode);
				else
					subTreeRootAfterRotation = rotateRight(parentNode, traversalNode);
			}
			else
			{
				--parentNode->internalAvlBalancingFactor;
				if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::BALANCED)
					break;

				traversalNode = parentNode;
				continue;
			}
		}
		else
		{
			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
			{
				if (traversalNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
					subTreeRootAfterRotation = rotateRightLeft(parentNode, traversalNode);
				else
					subTreeRootAfterRotation = rotateLeft(parentNode, traversalNode);
			}
			else
			{
				++parentNode->internalAvlBalancingFactor;
				if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::BALANCED)
					break;

				traversalNode = parentNode;
				continue;
			}
		}

		subTreeRootAfterRotation->parent = grandParentNode;
		if (grandParentNode)
		{
			if (grandParentNode->left == parentNode)
				grandParentNode->left = subTreeRootAfterRotation;
			else
				grandParentNode->right = subTreeRootAfterRotation;
		}
		else
			root = subTreeRootAfterRotation;

		break;
	}
	return true;
}


std::vector<std::size_t> AvlIntervalTree::getOverlappingIntervalsForLiteral(long literal) const
{
	if (!root || !literal)
		return {};

	std::vector<std::size_t> intersectedClauses;
	AvlIntervalTreeNode::ptr traversalNode = root;

	bool advancingDownLeftChildTree = false;
	while (traversalNode)
	{
		if (traversalNode->doesClauseIntersect(dimacs::ProblemDefinition::Clause::LiteralBounds(literal, literal)))
		{
			const std::vector<std::size_t>& intersectedClausesInNode = literal < traversalNode->key ? traversalNode->getIntersectedClauseIndicesMovingFromSmallestLowerBoundToMidPoint(literal) : traversalNode->getIntersectedClauseIndicesMovingLargestUpperBoundToMidPoint(literal);
			if (advancingDownLeftChildTree)
				intersectedClauses.insert(intersectedClauses.cbegin(), intersectedClausesInNode.cbegin(), intersectedClausesInNode.cend());
			else
				intersectedClauses.insert(intersectedClauses.cend(), intersectedClausesInNode.cbegin(), intersectedClausesInNode.cend());
		}

		if (literal < traversalNode->key)
		{
			traversalNode = traversalNode->left;
			advancingDownLeftChildTree = true;
		}
		else
		{
			traversalNode = traversalNode->right;
			advancingDownLeftChildTree = false;
		}
	}
	return intersectedClauses;
}

// TODO: IMPLEMENT ME
// see further: https://kukuruku.co/hub/cpp/avl-trees ???
// 

AvlIntervalTreeNode::ClauseRemovalResult AvlIntervalTree::removeClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (literalBounds.smallestLiteral > literalBounds.largestLiteral || !literalBounds.smallestLiteral || !literalBounds.largestLiteral)
		return AvlIntervalTreeNode::ClauseRemovalResult::ValidationError;

	const std::optional<avl::AvlIntervalTreeNode::ptr>& treeNodeContainingMatchingLiteralBounds = findNodeContainingLiteralBoundsOfClause(root, clauseIdxInFormula, literalBounds);
	if (!treeNodeContainingMatchingLiteralBounds)
		return AvlIntervalTreeNode::ClauseRemovalResult::NotFound;

	const AvlIntervalTreeNode::ClauseRemovalResult clauseRemovalResult = treeNodeContainingMatchingLiteralBounds->get()->removeIntersectedClause(clauseIdxInFormula, literalBounds);
	if (clauseRemovalResult == AvlIntervalTreeNode::ClauseRemovalResult::ValidationError)
		return clauseRemovalResult;

	if (clauseRemovalResult == AvlIntervalTreeNode::ClauseRemovalResult::NotFound)
		return clauseRemovalResult;

	if (treeNodeContainingMatchingLiteralBounds->get()->doesNodeStoreAnyInterval())
		return AvlIntervalTreeNode::ClauseRemovalResult::Removed;

	AvlIntervalTreeNode::ptr traversalNode = *treeNodeContainingMatchingLiteralBounds;
	AvlIntervalTreeNode::ptr leftChildOfNodeToDelete = traversalNode->left;
	AvlIntervalTreeNode::ptr inorderSuccessorOfNodeToDelete = findInorderSuccessorOfNode(traversalNode->right);

	AvlIntervalTreeNode::ptr replacementForDeletedNode = inorderSuccessorOfNodeToDelete ? inorderSuccessorOfNodeToDelete : leftChildOfNodeToDelete;
	std::optional<bool> backupOfLeftChildStatusFlagAtActionPosition;
	const bool isToBeDeletedNodeRoot = traversalNode == root;

	if (AvlIntervalTreeNode::ptr parentNodeOfToBeDeleteNode = traversalNode->parent; parentNodeOfToBeDeleteNode)
	{
		if (parentNodeOfToBeDeleteNode->left == traversalNode)
		{
			parentNodeOfToBeDeleteNode->left = replacementForDeletedNode;
			backupOfLeftChildStatusFlagAtActionPosition = true;
			//--parentNodeOfToBeDeleteNode->internalAvlBalancingFactor;
		}
		else
		{
			parentNodeOfToBeDeleteNode->right = replacementForDeletedNode;
			backupOfLeftChildStatusFlagAtActionPosition = false;
			//++parentNodeOfToBeDeleteNode->internalAvlBalancingFactor;
		}
	}
	
	
	// To be deleted node had only one child which is its left node
	if (leftChildOfNodeToDelete && replacementForDeletedNode == leftChildOfNodeToDelete)
	{
		leftChildOfNodeToDelete->parent = traversalNode->parent;
		// Rebalancing needs to start at the parent of the to be deleted node if the latter had only one child and said child being its left node
		traversalNode = traversalNode->parent;
	}
	else if (inorderSuccessorOfNodeToDelete && replacementForDeletedNode == inorderSuccessorOfNodeToDelete)
	{
		// Rebalancing needs to start at the parent of the in-order successor of the to be deleted node if the latter had to children 
		AvlIntervalTreeNode::ptr backupOfActionPosition = inorderSuccessorOfNodeToDelete->parent;
		// To be deleted node had only one child node namely its right node or the in-order successor was the right node of the node to be deleted
		if (inorderSuccessorOfNodeToDelete == traversalNode->right)
		{
			traversalNode->right = nullptr;
			AvlIntervalTreeNode::substituteNodeButKeepKey(*traversalNode, inorderSuccessorOfNodeToDelete);
			backupOfActionPosition = inorderSuccessorOfNodeToDelete;
		}
		else
		{
			// Swap the in-order successor with its right subtree in the left node of its parent node
			inorderSuccessorOfNodeToDelete->parent->left = inorderSuccessorOfNodeToDelete->right;
			AvlIntervalTreeNode::substituteNodeButKeepKey(*traversalNode, inorderSuccessorOfNodeToDelete);
			backupOfLeftChildStatusFlagAtActionPosition = true;
		}
		traversalNode = backupOfActionPosition;
	}
	else
	{
		// Rebalancing needs to start at the parent of the to be deleted node if the latter had no children
		traversalNode = traversalNode->parent;
	}

	if (isToBeDeletedNodeRoot)
		root = replacementForDeletedNode;

	// Rebalancing loop requires that the balancing factor of the node at the action position (i.e. the parent of the to be deleted node or the parent of its in-order successor) has not been updated yet.
	for (AvlIntervalTreeNode::ptr currProcessedNode = traversalNode; currProcessedNode; currProcessedNode = traversalNode->parent)
	{
		const AvlIntervalTreeNode::ptr parentNodeOfCurrProcessedNode = currProcessedNode->parent;
		if ((backupOfLeftChildStatusFlagAtActionPosition.has_value() && *backupOfLeftChildStatusFlagAtActionPosition) || traversalNode == currProcessedNode->left)
		{
			backupOfLeftChildStatusFlagAtActionPosition.reset();
			if (currProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
			{
				// Required rotation operation depends on balancing factor of right child of currently processed node
				AvlIntervalTreeNode::ptr rightChildOfCurrProcessedNode = currProcessedNode->right;
				if (rightChildOfCurrProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
					traversalNode = rotateRightLeft(currProcessedNode, rightChildOfCurrProcessedNode);
				else if (rightChildOfCurrProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
					traversalNode = rotateLeft(currProcessedNode, rightChildOfCurrProcessedNode);
				else
					break;
			}
			else if (currProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
			{
				// Deletion of left child changes balancing factor of currently processed node from LEFT_HEAVY to BALANCED but processing must continue since its grandparent node could now be unbalanced
				++currProcessedNode->internalAvlBalancingFactor;
				traversalNode = currProcessedNode;
			}
			else
			{
				// Balancing factor of currently processed node was previously BALANCED but due to the deletion of its left child will now be RIGHT_HEAVY but processing can stop now.
				++currProcessedNode->internalAvlBalancingFactor;
				break;
			}
		}
		else
		{
			backupOfLeftChildStatusFlagAtActionPosition.reset();
			if (currProcessedNode->internalAvlBalancingFactor == avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
			{
				// Required rotation operation depends on balancing factor of right child of currently processed node
				AvlIntervalTreeNode::ptr leftChildOfCurrProcessedNode = currProcessedNode->left;
				if (leftChildOfCurrProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
					traversalNode = rotateLeftRight(currProcessedNode, leftChildOfCurrProcessedNode);
				else if (leftChildOfCurrProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
					traversalNode = rotateRight(currProcessedNode, leftChildOfCurrProcessedNode);
				else
					break;
			}
			else if (currProcessedNode->internalAvlBalancingFactor == AvlIntervalTreeNode::RIGHT_HEAVY)
			{
				// Deletion of right child changes balancing factor of currently processed node from RIGHT_HEAVY to BALANCED but processing must continue since its grandparent node could now be unbalanced
				--currProcessedNode->internalAvlBalancingFactor;
				traversalNode = currProcessedNode;
			}
			else
			{
				// Balancing factor of currently processed node was previously BALANCED but due to the deletion of its right child will now be LEFT_HEAVY but processing can stop now.
				--currProcessedNode->internalAvlBalancingFactor;
				break;
			}
		}

		traversalNode->parent = parentNodeOfCurrProcessedNode;
		if (parentNodeOfCurrProcessedNode)
		{
			if (parentNodeOfCurrProcessedNode->left == currProcessedNode)
				parentNodeOfCurrProcessedNode->left = traversalNode;
			else
				parentNodeOfCurrProcessedNode->right = traversalNode;
		}
		else
		{
			root = traversalNode;
		}
	}
	return AvlIntervalTreeNode::ClauseRemovalResult::Removed;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::findNodeWithKey(long key) const
{
	AvlIntervalTreeNode::ptr node = root;
	while (node)
	{
		if (node->key == key)
			return node;
		node = key < node->key ? node->left : node->right;
	}
	return node;
}

//bool AvlIntervalTree::removeClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
//{
//	if (!root || literalBounds.smallestLiteral > literalBounds.largestLiteral || !literalBounds.smallestLiteral || !literalBounds.largestLiteral)
//		return false;
//
//	AvlIntervalTreeNode::ptr traversalNode = root;
//	while (traversalNode)
//	{
//		// TODO: Update of balancing factor of traversalNode?
//		// TODO: Does the median also have to be updated? For now we assume not
//		if (traversalNode->doesClauseIntersect(literalBounds))
//		{
//			if (!traversalNode->removeIntersectedClause(clauseIdxInFormula))
//				return false;
//
//			if (traversalNode->doesNodeStoreAnyInterval())
//				return true;
//
//			if (!traversalNode->left || !traversalNode->right)
//			{
//				AvlIntervalTreeNode::ptr onlyChildOfTraversalNode = traversalNode->left ? traversalNode->left : traversalNode->right;
//				if (!onlyChildOfTraversalNode)
//				{
//					// DELETED NODE HAD NO CHILDREN
//					if (traversalNode == root)
//					{
//						root = nullptr;
//						return true;
//					}
//					if (traversalNode->parent->left == traversalNode)
//					{
//						traversalNode->parent->left = nullptr;
//						//++traversalNode->parent->internalAvlBalancingFactor;
//					}
//					else
//					{
//						traversalNode->parent->right = nullptr;
//						//--traversalNode->parent->internalAvlBalancingFactor;
//					}
//				}
//				else
//				{
//					if (traversalNode == root)
//					{
//						onlyChildOfTraversalNode->parent = nullptr;
//						root = onlyChildOfTraversalNode;
//						return true;
//					}
//
//					// DELETED NODE HAD ONE CHILD
//					onlyChildOfTraversalNode->parent = traversalNode->parent;
//					if (traversalNode->parent->left == traversalNode)
//						traversalNode->parent->left = onlyChildOfTraversalNode;
//					else
//						traversalNode->parent->right = onlyChildOfTraversalNode;
//					// TODO:
//					traversalNode->parent->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
//					//traversalNode = onlyChildOfTraversalNode;
//				}
//				//traversalNode = traversalNode->parent;
//			}
//			else
//			{
//				// TODO: Inorder successor replaces current node
//				// TO BE DELETED NODE HAS TWO CHILDREN - In order successor of right child needs to be chosen
//				AvlIntervalTreeNode::ptr inorderSuccessorInRightChild = traversalNode->right;
//				while (inorderSuccessorInRightChild->left)
//				{
//					inorderSuccessorInRightChild = inorderSuccessorInRightChild->left;
//				}
//
//				AvlIntervalTreeNode::ptr backupOfStartForRetracingOperation = inorderSuccessorInRightChild->parent;
//				if (backupOfStartForRetracingOperation->left == inorderSuccessorInRightChild)
//				{
//					++backupOfStartForRetracingOperation->internalAvlBalancingFactor;
//					backupOfStartForRetracingOperation->left = nullptr;
//				}
//				else
//				{
//					--backupOfStartForRetracingOperation->internalAvlBalancingFactor;
//					backupOfStartForRetracingOperation->right = nullptr;
//				}
//
//				inorderSuccessorInRightChild->parent = traversalNode->parent;
//				if (traversalNode->parent)
//				{
//					if (traversalNode->parent->left == traversalNode)
//						traversalNode->parent->left = inorderSuccessorInRightChild;
//					else
//						traversalNode->parent->right = inorderSuccessorInRightChild;
//				}
//
//				if (traversalNode->left)
//				{
//					traversalNode->left->parent = inorderSuccessorInRightChild;
//					inorderSuccessorInRightChild->left = traversalNode->left;
//				}
//				traversalNode->right->parent = inorderSuccessorInRightChild;
//				inorderSuccessorInRightChild->right = traversalNode->right;
//				inorderSuccessorInRightChild->parent = nullptr;
//				if (traversalNode == root)
//					root = inorderSuccessorInRightChild;
//
//				traversalNode = backupOfStartForRetracingOperation;
//			}
//			break;
//		}
//		if (literalBounds.smallestLiteral < traversalNode->getSmallestLowerBound().value_or(0))
//			traversalNode = traversalNode->left;
//		else
//			traversalNode = traversalNode->right;
//	}
//
//	AvlIntervalTreeNode::BalancingFactor balancingFactorOfChildNodePriorToRotation = AvlIntervalTreeNode::BalancingFactor::BALANCED;
//	for (AvlIntervalTreeNode::ptr parentNode = traversalNode->parent; parentNode; parentNode = traversalNode->parent)
//	{
//		AvlIntervalTreeNode::ptr grandParentNode = parentNode->parent;
//		AvlIntervalTreeNode::ptr subTreeRootNode = nullptr;
//		if (traversalNode == parentNode->left)
//		{
//			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
//			{
//				balancingFactorOfChildNodePriorToRotation = parentNode->right->internalAvlBalancingFactor;
//				if (parentNode->right->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
//					subTreeRootNode = rotateRightLeft(parentNode, parentNode->right);
//				else
//					subTreeRootNode = rotateLeft(parentNode, parentNode->right);
//			}
//			else if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
//			{
//				parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
//				traversalNode = parentNode;
//				continue;
//			}
//			else
//			{
//				--parentNode->internalAvlBalancingFactor;
//				break;
//			}
//		}
//		else
//		{
//			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
//			{
//				balancingFactorOfChildNodePriorToRotation = parentNode->left->internalAvlBalancingFactor;
//				if (parentNode->left->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
//					subTreeRootNode = rotateLeftRight(parentNode, parentNode->left);
//				else
//					subTreeRootNode = rotateRight(parentNode, parentNode->left);
//			}
//			else if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
//			{
//				parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
//				traversalNode = parentNode;
//				continue;
//			}
//			else
//			{
//				++parentNode->internalAvlBalancingFactor;
//				break;
//			}
//		}
//
//		subTreeRootNode->parent = grandParentNode;
//		if (grandParentNode)
//		{
//			if (grandParentNode->left == parentNode)
//				grandParentNode->left = subTreeRootNode;
//			else
//				grandParentNode->right = subTreeRootNode;
//		}
//		else
//			root = subTreeRootNode;
//
//		// We can stop the retracing (re-balancing) if the new subtree root, after a performed rotation, is the left/right child of the original parent node since the child nodes of the previously balanced node are "distributed" in the rotated nodes
//		if (balancingFactorOfChildNodePriorToRotation == AvlIntervalTreeNode::BalancingFactor::BALANCED)
//			break;
//	}
//	return true;
//}

// BEGIN NON-PUBLIC FUNCTIONALITY
std::optional<AvlIntervalTreeNode::ptr> AvlIntervalTree::findNodeContainingLiteralBoundsOfClause(const AvlIntervalTreeNode::ptr& searchStartingPositionInTree, std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (!searchStartingPositionInTree)
		return std::nullopt;

	const AvlIntervalTreeNode::ptr& traversalNode = searchStartingPositionInTree;
	std::optional<AvlIntervalTreeNode::ptr> matchingNodeForLiteralBounds = traversalNode->doesClauseIntersect(literalBounds) && traversalNode->doesNodeContainMatchingLiteralBoundsForClause(clauseIdxInFormula, literalBounds) ? std::make_optional(traversalNode) : std::nullopt;

	if (!matchingNodeForLiteralBounds.has_value() && literalBounds.smallestLiteral < traversalNode->key)
		matchingNodeForLiteralBounds = findNodeContainingLiteralBoundsOfClause(traversalNode->left, clauseIdxInFormula, literalBounds);

	if (!matchingNodeForLiteralBounds.has_value() && literalBounds.largestLiteral > traversalNode->key)
		matchingNodeForLiteralBounds = findNodeContainingLiteralBoundsOfClause(traversalNode->right, clauseIdxInFormula, literalBounds);

	return matchingNodeForLiteralBounds;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::rotateLeft(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& rightChild)
{
	parentNode->right = rightChild->left;
	if (rightChild->left)
		rightChild->left->parent = parentNode;

	parentNode->parent = rightChild;
	rightChild->left = parentNode;

	rightChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;

	return rightChild;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::rotateRight(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& leftChild)
{
	parentNode->left = leftChild->right;
	if (leftChild->right)
		leftChild->right->parent = parentNode;

	parentNode->parent = leftChild;
	leftChild->right = parentNode;

	leftChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;

	return leftChild;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::rotateLeftRight(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& leftChild)
{
	AvlIntervalTreeNode::ptr nodeZ = leftChild->right;
	const AvlIntervalTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlIntervalTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

	nodeZ->left = leftChild;
	nodeZ->right = parentNode;

	//leftChild->parent = nodeZ;
	//leftChild->right = rightChildOfNodeZ;

	//if (rightChildOfNodeZ)
	//	rightChildOfNodeZ->parent = parentNode;

	//parentNode->left = leftChildOfNodeZ;
	//if (leftChildOfNodeZ)
	//	leftChildOfNodeZ->parent = parentNode;

	leftChild->parent = nodeZ;
	leftChild->right = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = leftChild;

	parentNode->parent = nodeZ;
	parentNode->left = rightChildOfNodeZ;
	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = parentNode;


	if (nodeZ->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::BALANCED)
	{
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
		leftChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	}
	else if (nodeZ->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
	{
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
		leftChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	}
	else
	{
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
		leftChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
	}
	nodeZ->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	return nodeZ;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::rotateRightLeft(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& rightChild)
{
	AvlIntervalTreeNode::ptr nodeZ = rightChild->left;
	const AvlIntervalTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlIntervalTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

	nodeZ->right = rightChild;
	nodeZ->left = parentNode;

	rightChild->parent = nodeZ;
	rightChild->left = rightChildOfNodeZ;
	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = rightChild;

	parentNode->right = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = parentNode;

	parentNode->parent = nodeZ;

	if (nodeZ->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::BALANCED)
	{
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
		rightChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	}
	else if (nodeZ->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
	{
		rightChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	}
	else
	{
		parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
		rightChild->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	}
	nodeZ->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	return nodeZ;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::findInorderSuccessorOfNode(const AvlIntervalTreeNode::ptr& node)
{
	if (!node)
		return node;

	AvlIntervalTreeNode::ptr currProcessedNode = node;
	while (currProcessedNode->left)
	{
		currProcessedNode = currProcessedNode->left;
	}
	return currProcessedNode;
}
