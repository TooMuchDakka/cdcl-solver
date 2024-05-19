#include "optimizations/avlTree.hpp"

#include <cassert>

using namespace avl;

bool AvlTree::find(long literal) const
{
	AvlTreeNode::ptr traversalNode = root;
	while (traversalNode)
	{
		if (traversalNode->key == literal)
			return true;

		if (literal < traversalNode->key)
			traversalNode = traversalNode->left;
		else
			traversalNode = traversalNode->right;
	}
	return false;
}

bool AvlTree::insert(long literal)
{
	const AvlTreeNode::ptr nodeToInsert = std::make_shared<AvlTreeNode>(literal);

	if (!nodeToInsert)
		return false;

	if (!root)
	{
		root = nodeToInsert;
		return true;
	}
	
	AvlTreeNode::ptr traversalNode = root;
	while (traversalNode)
	{
		if (literal > traversalNode->key)
		{
			if (traversalNode->right)
			{
				traversalNode = traversalNode->right;
			}
			else
			{
				traversalNode->right = nodeToInsert;
				nodeToInsert->parent = traversalNode;
				traversalNode = nullptr;
			}
		}
		else if (literal < traversalNode->key)
		{
			if (traversalNode->left)
			{
				traversalNode = traversalNode->left;
			}
			else
			{
				traversalNode->left = nodeToInsert;
				nodeToInsert->parent = traversalNode;
				traversalNode = nullptr;
			}
		}
		else
		{
			return traversalNode->incrementReferenceCount();
		}
	}

	traversalNode = nodeToInsert;
	AvlTreeNode::ptr grandParent = nullptr;
	AvlTreeNode::ptr newSubtreeRoot = nullptr;

	for (AvlTreeNode::ptr parentNode = traversalNode->parent; parentNode; parentNode = traversalNode->parent)
	{
		if (parentNode->left == traversalNode)
		{
			// Rebalancing required due to the parent node already being left heavy
			if (parentNode->balancingFactor == AvlTreeNode::LEFT_HEAVY)
			{
				grandParent = parentNode->parent;
				if (traversalNode->balancingFactor == AvlTreeNode::RIGHT_HEAVY)
					newSubtreeRoot = rotateLeftRight(parentNode, traversalNode);
				else
					newSubtreeRoot = rotateRight(parentNode, traversalNode);

				if (!doesNodeInvariantHold(*newSubtreeRoot))
					int x = 0;

				assertNodeInvariant(*newSubtreeRoot);
			}
			else
			{
				/*if (parentNode->balancingFactor == AvlTreeNode::RIGHT_HEAVY)
				{
					parentNode->balancingFactor = AvlTreeNode::BALANCED;
					break;
				}
				
				parentNode->balancingFactor = AvlTreeNode::LEFT_HEAVY;*/

				--parentNode->balancingFactor;
				if (parentNode->balancingFactor == AvlTreeNode::BALANCED)
					break;

				traversalNode = parentNode;
				continue;
			}
		}
		else
		{
			if (parentNode->balancingFactor == AvlTreeNode::RIGHT_HEAVY)
			{
				grandParent = parentNode->parent;
				if (traversalNode->balancingFactor == AvlTreeNode::LEFT_HEAVY)
					newSubtreeRoot = rotateRightLeft(parentNode, traversalNode);
				else
					newSubtreeRoot = rotateLeft(parentNode, traversalNode);

				if (!doesNodeInvariantHold(*newSubtreeRoot))
					int x = 0;

				assertNodeInvariant(*newSubtreeRoot);

			}
			else
			{
				/*if (parentNode->balancingFactor == AvlTreeNode::LEFT_HEAVY)
				{
					parentNode->balancingFactor = AvlTreeNode::BALANCED;
					break;
				}

				parentNode->balancingFactor = AvlTreeNode::RIGHT_HEAVY;*/

				++parentNode->balancingFactor;
				if (parentNode->balancingFactor == AvlTreeNode::BALANCED)
					break;

				traversalNode = parentNode;
				continue;
			}
		}

		newSubtreeRoot->parent = grandParent;
		if (grandParent)
		{
			if (grandParent->left == parentNode)
				grandParent->left = newSubtreeRoot;
			else
				grandParent->right = newSubtreeRoot;
		}
		else
		{
			root = newSubtreeRoot;
		}
		break;
	}
	return true;
}

// TODO: Rebalancing loop
bool AvlTree::remove(long literal)
{
	if (!root)
		return true;

	AvlTreeNode::ptr traversalNode = root;
	while (traversalNode)
	{
		if (literal < traversalNode->key)
			traversalNode = traversalNode->left;
		else if (literal > traversalNode->key)
			traversalNode = traversalNode->right;
		else
			break;
	}

	if (!traversalNode->decrementReferenceCount())
		return false;

	if (traversalNode->getReferenceCount())
		return true;

	if (traversalNode == root)
	{
		root = nullptr;
		return true;
	}

	// BEGIN ACTUAL DELETION OF NODE
	AvlTreeNode::ptr replacementForTraversalNode;
	AvlTreeNode::ptr startPointForRebalancingSearch;
	//const bool isTraversalNodeLeftChild = traversalNode == traversalNode->parent->left;

	//bool skipUpdateOfBalancingFactor = false;
	if (!traversalNode->left && !traversalNode->right)
	{
		replacementForTraversalNode = nullptr;
	}
	else if (!traversalNode->right)
	{
		replacementForTraversalNode = traversalNode->left;
	}
	else if (!traversalNode->left)
	{
		replacementForTraversalNode = traversalNode->right;
	}
	else
	{
		AvlTreeNode::ptr inorderSuccessorOfTraversalNode = traversalNode;
		while (inorderSuccessorOfTraversalNode->left)
			inorderSuccessorOfTraversalNode = inorderSuccessorOfTraversalNode->left;

		inorderSuccessorOfTraversalNode->left = traversalNode->left;
		// TODO:
		++inorderSuccessorOfTraversalNode->parent->balancingFactor;
		traversalNode->left->parent = inorderSuccessorOfTraversalNode;
		
		if (inorderSuccessorOfTraversalNode != traversalNode->right)
			replaceNodeInAvlTreeStructure(inorderSuccessorOfTraversalNode == inorderSuccessorOfTraversalNode->parent->left ? inorderSuccessorOfTraversalNode->parent->left : inorderSuccessorOfTraversalNode->parent->right, nullptr);
		replacementForTraversalNode = inorderSuccessorOfTraversalNode;
		//skipUpdateOfBalancingFactor = true;
		//startPointForRebalancingSearch = inorderSuccessorOfTraversalNode->parent;
	}

	/*if (!skipUpdateOfBalancingFactor)
	{
		if (isTraversalNodeLeftChild)
			--traversalNode->parent->balancingFactor;
		else
			++traversalNode->parent->balancingFactor;
	}*/
	
	replaceNodeInAvlTreeStructure(traversalNode, replacementForTraversalNode);
	traversalNode = replacementForTraversalNode;
	if (!replacementForTraversalNode)
		traversalNode = traversalNode->parent;

	if (startPointForRebalancingSearch)
		traversalNode = startPointForRebalancingSearch;

	// END ACTUAL DELETION OF NODE

	AvlTreeNode::ptr grandParentNode;
	for (AvlTreeNode::ptr parentNode = traversalNode->parent; parentNode; parentNode = grandParentNode)
	{
		grandParentNode = parentNode->parent;
		AvlTreeNode::ptr newSubtreeRoot;
		if (parentNode->left->key == literal)
		{
			if (parentNode->balancingFactor == AvlTreeNode::RIGHT_HEAVY)
			{
				AvlTreeNode::ptr rightChildOfCurrentNode = traversalNode->right;
				if (rightChildOfCurrentNode->balancingFactor == AvlTreeNode::LEFT_HEAVY)
					newSubtreeRoot = rotateRightLeft(traversalNode, rightChildOfCurrentNode);
				else
					newSubtreeRoot = rotateLeft(traversalNode, rightChildOfCurrentNode);
			}
			else if (parentNode->balancingFactor == AvlTreeNode::BALANCED)
			{
				parentNode->balancingFactor = AvlTreeNode::RIGHT_HEAVY;
				break;
			}
			else
			{
				parentNode->balancingFactor = AvlTreeNode::BALANCED;
				traversalNode = parentNode;
			}
		}
		else
		{
			if (parentNode->balancingFactor == AvlTreeNode::LEFT_HEAVY)
			{
				AvlTreeNode::ptr leftChildOfCurrentNode = traversalNode->left;
				if (leftChildOfCurrentNode->balancingFactor == AvlTreeNode::RIGHT_HEAVY)
					newSubtreeRoot = rotateLeftRight(traversalNode, leftChildOfCurrentNode);
				else
					newSubtreeRoot = rotateRight(traversalNode, leftChildOfCurrentNode);
			}
			else if (parentNode->balancingFactor == AvlTreeNode::BALANCED)
			{
				parentNode->balancingFactor = AvlTreeNode::LEFT_HEAVY;
				break;
			}
			else
			{
				parentNode->balancingFactor = AvlTreeNode::BALANCED;
				traversalNode = parentNode;
			}
		}

		newSubtreeRoot->parent = grandParentNode;
		if (grandParentNode)
		{
			if (grandParentNode->left == parentNode)
				grandParentNode->left = newSubtreeRoot;
			else
				grandParentNode->right = newSubtreeRoot;
		}
		else
			root = newSubtreeRoot;

		parentNode = nullptr;
	}
	
	return false;
}

// BEGIN NON-PUBLIC FUNCTIONALITY
AvlTree::AvlTreeNode::ptr AvlTree::rotateLeft(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& rightChild)
{
	parentNode->right = rightChild->left;
	if (rightChild->left)
		rightChild->left->parent = parentNode;

	parentNode->parent = rightChild;
	rightChild->left = parentNode;

	rightChild->balancingFactor = AvlTreeNode::BALANCED;
	parentNode->balancingFactor = AvlTreeNode::BALANCED;
	return rightChild;
}

AvlTree::AvlTreeNode::ptr AvlTree::rotateLeftRight(const AvlTreeNode::ptr& parentNode, const  AvlTreeNode::ptr& leftChild)
{
	/*AvlTreeNode::ptr nodeZ = leftChild->right;
	const AvlTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

	nodeZ->parent = nullptr;
	nodeZ->left = leftChild;
	nodeZ->right = parentNode;

	leftChild->parent = nodeZ;
	leftChild->right = rightChildOfNodeZ;
	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = parentNode;

	parentNode->left = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = parentNode;

	parentNode->parent = nodeZ;
	
	if (nodeZ->balancingFactor == AvlTreeNode::BALANCED)
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		leftChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else if (nodeZ->balancingFactor == AvlTreeNode::LEFT_HEAVY)
	{
		parentNode->balancingFactor = AvlTreeNode::RIGHT_HEAVY;
		leftChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		leftChild->balancingFactor = AvlTreeNode::LEFT_HEAVY;
	}
	nodeZ->balancingFactor = AvlTreeNode::BALANCED;
	return nodeZ;*/

	AvlTreeNode::ptr nodeZ = leftChild->right;
	const AvlTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

	nodeZ->left = leftChild;
	nodeZ->right = parentNode;

	leftChild->parent = nodeZ;
	leftChild->right = rightChildOfNodeZ;
	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = parentNode;

	parentNode->left = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = parentNode;

	parentNode->parent = nodeZ;

	if (nodeZ->balancingFactor == AvlTreeNode::BALANCED)
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		leftChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else if (nodeZ->balancingFactor == AvlTreeNode::LEFT_HEAVY)
	{
		parentNode->balancingFactor = AvlTreeNode::RIGHT_HEAVY;
		leftChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		leftChild->balancingFactor = AvlTreeNode::LEFT_HEAVY;
	}
	nodeZ->balancingFactor = AvlTreeNode::BALANCED;
	return nodeZ;
}

AvlTree::AvlTreeNode::ptr AvlTree::rotateRight(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& leftChild)
{
	parentNode->left = leftChild->right;
	if (leftChild->right)
		leftChild->right->parent = parentNode;

	parentNode->parent = leftChild;
	leftChild->right = parentNode;

	leftChild->balancingFactor = AvlTreeNode::BALANCED;
	parentNode->balancingFactor = AvlTreeNode::BALANCED;
	return leftChild;
}

AvlTree::AvlTreeNode::ptr AvlTree::rotateRightLeft(const AvlTreeNode::ptr& parentNode, const AvlTreeNode::ptr& rightChild)
{
	/*AvlTreeNode::ptr nodeZ = rightChild->left;
	const AvlTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

	nodeZ->parent = nullptr;
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

	if (nodeZ->balancingFactor == AvlTreeNode::BALANCED)
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		rightChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else if (nodeZ->balancingFactor == AvlTreeNode::LEFT_HEAVY)
	{
		rightChild->balancingFactor = AvlTreeNode::RIGHT_HEAVY;
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
	}
	else
	{
		parentNode->balancingFactor = AvlTreeNode::LEFT_HEAVY;
		rightChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	nodeZ->balancingFactor = AvlTreeNode::BALANCED;
	return nodeZ;*/


	AvlTreeNode::ptr nodeZ = rightChild->left;
	const AvlTreeNode::ptr leftChildOfNodeZ = nodeZ->left;
	const AvlTreeNode::ptr rightChildOfNodeZ = nodeZ->right;

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

	if (nodeZ->balancingFactor == AvlTreeNode::BALANCED)
	{
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
		rightChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	else if (nodeZ->balancingFactor == AvlTreeNode::LEFT_HEAVY)
	{
		rightChild->balancingFactor = AvlTreeNode::RIGHT_HEAVY;
		parentNode->balancingFactor = AvlTreeNode::BALANCED;
	}
	else
	{
		parentNode->balancingFactor = AvlTreeNode::LEFT_HEAVY;
		rightChild->balancingFactor = AvlTreeNode::BALANCED;
	}
	nodeZ->balancingFactor = AvlTreeNode::BALANCED;
	return nodeZ; 
}

void AvlTree::replaceNodeInAvlTreeStructure(const AvlTreeNode::ptr& nodeToReplace, const AvlTreeNode::ptr& replacementNode)
{
	const AvlTreeNode::ptr& parentNodeOfNodeToReplace = nodeToReplace->parent;
	if (nodeToReplace == parentNodeOfNodeToReplace->left)
	{
		parentNodeOfNodeToReplace->left = replacementNode;
		
	}
	else if (nodeToReplace == parentNodeOfNodeToReplace->right)
	{
		parentNodeOfNodeToReplace->right = replacementNode;
	}
	else
	{
		return;
	}

	if (replacementNode)
		replacementNode->parent = parentNodeOfNodeToReplace;
}

void AvlTree::assertNodeInvariant(const AvlTreeNode& treeNode)
{
	/*if ((treeNode.left && treeNode.key < treeNode.left->key) || (treeNode.right && treeNode.key > treeNode.right->key))
	{
		int x = 0;
	}

	if (treeNode.left)
		assert(treeNode.key > treeNode.left->key);

	if (treeNode.right)
		assert(treeNode.key < treeNode.right->key);*/
}

bool AvlTree::doesNodeInvariantHold(const AvlTreeNode& treeNode)
{
	bool doesInvariantHold = true;
	if (treeNode.left)
		doesInvariantHold = treeNode.key > treeNode.left->key;

	if (treeNode.right)
		doesInvariantHold = treeNode.key < treeNode.right->key;
	return doesInvariantHold;
}


// TODO: Check operations for literals bounds and clauses (contains smaller, larger literal, etc.)