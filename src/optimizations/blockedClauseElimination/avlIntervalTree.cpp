#include "optimizations/blockedClauseElimination/avlIntervalTree.hpp"

using namespace avl;

bool AvlIntervalTree::insertClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (literalBounds.smallestLiteral > literalBounds.largestLiteral || !literalBounds.smallestLiteral || !literalBounds.largestLiteral)
		return false;

	const long keyOfNewNode = literalBounds.smallestLiteral;
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
		if (traversalNode->doesClauseIntersect(literalBounds))
		{
			traversalNode->insertLowerBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.smallestLiteral, clauseIdxInFormula));
			traversalNode->insertUpperBound(AvlIntervalTreeNode::LiteralBoundsAndClausePair(literalBounds.largestLiteral, clauseIdxInFormula));
			return true;
		}

		if (keyOfNewNode < traversalNode->getSmallestLowerBound().value_or(0))
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
bool AvlIntervalTree::removeClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (!root || literalBounds.smallestLiteral > literalBounds.largestLiteral || !literalBounds.smallestLiteral || !literalBounds.largestLiteral)
		return false;

	AvlIntervalTreeNode::ptr traversalNode = root;
	while (traversalNode)
	{
		// TODO: Update of balancing factor of traversalNode?
		// TODO: Does the median also have to be updated? For now we assume not
		if (traversalNode->doesClauseIntersect(literalBounds))
		{
			if (!traversalNode->removeIntersectedClause(clauseIdxInFormula))
				return false;

			if (traversalNode->doesNodeStoreAnyInterval())
				return true;

			if (!traversalNode->left || !traversalNode->right)
			{
				AvlIntervalTreeNode::ptr onlyChildOfTraversalNode = traversalNode->left ? traversalNode->left : traversalNode->right;
				if (!onlyChildOfTraversalNode)
				{
					// DELETED NODE HAD NO CHILDREN
					if (traversalNode == root)
					{
						root = nullptr;
						return true;
					}
					if (traversalNode->parent->left == traversalNode)
					{
						traversalNode->parent->left = nullptr;
						//++traversalNode->parent->internalAvlBalancingFactor;
					}
					else
					{
						traversalNode->parent->right = nullptr;
						//--traversalNode->parent->internalAvlBalancingFactor;
					}
				}
				else
				{
					// DELETED NODE HAD ONE CHILD
					onlyChildOfTraversalNode->parent = traversalNode->parent;
					if (traversalNode->parent->left == traversalNode)
						traversalNode->parent->left = onlyChildOfTraversalNode;
					else
						traversalNode->parent->right = onlyChildOfTraversalNode;

					traversalNode = onlyChildOfTraversalNode;
				}
			}
			else
			{
				// TODO: Inorder successor replaces current node
				// TO BE DELETED NODE HAS TWO CHILDREN - In order successor of right child needs to be chosen
				AvlIntervalTreeNode::ptr inorderSuccessorInRightChild = traversalNode->right;
				while (inorderSuccessorInRightChild->left)
				{
					inorderSuccessorInRightChild = inorderSuccessorInRightChild->left;
				}

				if (inorderSuccessorInRightChild->parent->left == inorderSuccessorInRightChild)
					inorderSuccessorInRightChild->parent->left = nullptr;
				else
					inorderSuccessorInRightChild->parent->right = nullptr;

				if (traversalNode->parent)
				{
					if (traversalNode->parent->left == traversalNode)
						traversalNode->parent->left = inorderSuccessorInRightChild;
					else
						traversalNode->parent->right = inorderSuccessorInRightChild;

					inorderSuccessorInRightChild->parent = traversalNode->parent;
				}
				else
				{
					root = inorderSuccessorInRightChild;
					inorderSuccessorInRightChild->parent = nullptr;
				}
				traversalNode = inorderSuccessorInRightChild;
			}
		}
		else if (literalBounds.smallestLiteral < traversalNode->getSmallestLowerBound().value_or(0))
			traversalNode = traversalNode->left;
		else
			traversalNode = traversalNode->right;
	}

	AvlIntervalTreeNode::BalancingFactor balancingFactorOfChildNodePriorToRotation = AvlIntervalTreeNode::BalancingFactor::BALANCED;
	for (AvlIntervalTreeNode::ptr parentNode = traversalNode->parent; parentNode; parentNode = traversalNode->parent)
	{
		AvlIntervalTreeNode::ptr grandParentNode = parentNode->parent;
		AvlIntervalTreeNode::ptr subTreeRootNode = nullptr;
		if (traversalNode == parentNode->left)
		{
			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
			{
				balancingFactorOfChildNodePriorToRotation = parentNode->right->internalAvlBalancingFactor;
				if (parentNode->right->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
					subTreeRootNode = rotateRightLeft(parentNode, parentNode->right);
				else
					subTreeRootNode = rotateLeft(parentNode, parentNode->right);
			}
			else if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
			{
				parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
				traversalNode = parentNode;
			}
			else
			{
				++parentNode->internalAvlBalancingFactor;
				break;
			}
		}
		else
		{
			if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY)
			{
				balancingFactorOfChildNodePriorToRotation = parentNode->left->internalAvlBalancingFactor;
				if (parentNode->left->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
					subTreeRootNode = rotateLeftRight(parentNode, parentNode->left);
				else
					subTreeRootNode = rotateRight(parentNode, parentNode->left);
			}
			else if (parentNode->internalAvlBalancingFactor == AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY)
			{
				parentNode->internalAvlBalancingFactor = AvlIntervalTreeNode::BalancingFactor::BALANCED;
				traversalNode = parentNode;
			}
			else
			{
				--parentNode->internalAvlBalancingFactor;
				break;
			}
		}

		subTreeRootNode->parent = grandParentNode;
		if (grandParentNode)
		{
			if (grandParentNode->left == parentNode)
				grandParentNode->left = subTreeRootNode;
			else
				grandParentNode->right = subTreeRootNode;
		}
		else
			root = subTreeRootNode;

		// We can stop the retracing (re-balancing) if the new subtree root, after a performed rotation, is the left/right child of the original parent node since the child nodes of the previously balanced node are "distributed" in the rotated nodes
		if (balancingFactorOfChildNodePriorToRotation == AvlIntervalTreeNode::BalancingFactor::BALANCED)
			break;
	}
	return true;
}

// BEGIN NON-PUBLIC FUNCTIONALITY
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

	leftChild->parent = nodeZ;
	leftChild->right = rightChildOfNodeZ;

	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = parentNode;

	parentNode->left = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = parentNode;

	parentNode->parent = nodeZ;

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