#include "optimizations/utils/avlIntervalTree.hpp"
#include "optimizations/utils/binarySearchUtils.hpp"

using namespace avl;

std::unordered_set<std::size_t> AvlIntervalTree::determineIndicesOfClausesContainingLiteral(long literal) const
{
	std::unordered_set<std::size_t> aggregateOfOVerlappingClauseIndices;

	AvlIntervalTreeNode::ptr currNode = avlTreeRoot;
	while (currNode)
	{
		if (currNode->intervalMidPoint <= literal)
		{
			const std::vector<std::size_t>& overlappingIndicesOfCurrNode = currNode->overlappingIntervalsLowerBoundsData.getIndicesOfClausesOverlappingLiteralBound(literal);
			currNode = currNode->intervalMidPoint == literal ? nullptr : currNode->left;
			if (!recordClausesContainingLiteral(*formula, literal, overlappingIndicesOfCurrNode, aggregateOfOVerlappingClauseIndices))
				return {};
		}
		else
		{
			const std::vector<std::size_t>& overlappingIndicesOfCurrNode = currNode->overlappingIntervalsUpperBoundsData.getIndicesOfClausesOverlappingLiteralBound(literal);
			currNode = currNode->right;
			if (!recordClausesContainingLiteral(*formula, literal, overlappingIndicesOfCurrNode, aggregateOfOVerlappingClauseIndices))
				return {};
		}
	}
	return aggregateOfOVerlappingClauseIndices;
}

bool AvlIntervalTree::insertClause(std::size_t clauseIndex, const dimacs::ProblemDefinition::Clause& clause)
{
	if (clause.literals.empty())
		return false;

	const long clauseLiteralsMidpoint = determineLiteralBoundsMidPoint(clause);
	auto newTreeNode = std::make_shared<AvlIntervalTreeNode>(clauseLiteralsMidpoint, nullptr);
	newTreeNode->overlappingIntervalsLowerBoundsData.insertClause(clauseIndex, *clause.getSmallestLiteralOfClause());
	newTreeNode->overlappingIntervalsUpperBoundsData.insertClause(clauseIndex, *clause.getLargestLiteralOfClause());

	if (!avlTreeRoot)
	{
		avlTreeRoot = newTreeNode;
		return true;
	}

	AvlIntervalTreeNode::ptr traversalNode = avlTreeRoot;
	while (traversalNode)
	{
		if (clauseLiteralsMidpoint >= traversalNode->getSmallestLiteralBoundOfOverlappedClauses() && clauseLiteralsMidpoint <= traversalNode->getLargestLiteralBoundOfOverlappedClauses())
		{
			traversalNode->overlappingIntervalsLowerBoundsData.insertClause(clauseIndex, *clause.getSmallestLiteralOfClause());
			traversalNode->overlappingIntervalsUpperBoundsData.insertClause(clauseIndex, *clause.getLargestLiteralOfClause());
			return true;
		}

		if (clauseLiteralsMidpoint < traversalNode->intervalMidPoint)
		{
			if (!traversalNode->left)
			{
				traversalNode->left = newTreeNode;
				newTreeNode->parent = traversalNode;
				traversalNode = nullptr;
			}
			else
			{
				traversalNode = traversalNode->left;
			}
		}
		else
		{
			if (!traversalNode->right)
			{
				traversalNode->right = newTreeNode;
				newTreeNode->parent = traversalNode;
				traversalNode = nullptr;
			}
			else
			{
				traversalNode = traversalNode->right;
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
			if (parentNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::LeftHeavy)
			{
				if (traversalNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::RightHeavy)
					subTreeRootAfterRotation = rotateLeftRight(parentNode, traversalNode);
				else
					subTreeRootAfterRotation = rotateRight(parentNode, traversalNode);
			}
			else
			{
				--parentNode->balancingFactor;
				if (parentNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::Balanced)
					break;

				traversalNode = parentNode;
				continue;
			}
		}
		else
		{
			if (parentNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::RightHeavy)
			{
				if (traversalNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::LeftHeavy)
					subTreeRootAfterRotation = rotateRightLeft(parentNode, traversalNode);
				else
					subTreeRootAfterRotation = rotateLeft(parentNode, traversalNode);
			}
			else
			{
				++parentNode->balancingFactor;
				if (parentNode->balancingFactor == AvlIntervalTreeNode::BalancingFactor::Balanced)
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
			avlTreeRoot = subTreeRootAfterRotation;

		break;
	}
	return true;
}

// BEGIN NON-PUBLIC INTERFACE IMPLEMENTATION
bool AvlIntervalTree::recordClausesContainingLiteral(const dimacs::ProblemDefinition& formula, long literal, const std::vector<std::size_t>& clauseIndices, std::unordered_set<std::size_t>& aggregatorOfClauseIndicesContainingLiteral)
{
	for (const std::size_t clauseIndex : clauseIndices)
	{
		const std::optional<const dimacs::ProblemDefinition::Clause*> referenceClause = formula.getClauseByIndexInFormula(clauseIndex);
		if (!referenceClause)
			return false;

		const std::vector<long>& clauseLiterals = referenceClause.value()->literals;
		if (!bSearchInSortedContainer(clauseLiterals, literal, bSearchUtils::SortOrder::Ascending))
			continue;
		aggregatorOfClauseIndicesContainingLiteral.emplace(clauseIndex);
	}
	return true;
}

long AvlIntervalTree::determineLiteralBoundsMidPoint(const dimacs::ProblemDefinition::Clause& clause)
{
	const long smallestLiteralOfClause = clause.getSmallestLiteralOfClause().value();
	const long largestLiteralOfClause = clause.getLargestLiteralOfClause().value();

	return smallestLiteralOfClause + ((largestLiteralOfClause - smallestLiteralOfClause) / 2);
}


AvlIntervalTreeNode::ptr AvlIntervalTree::rotateLeft(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& rightChild)
{
	parentNode->right = rightChild->left;
	if (rightChild->left)
		rightChild->left->parent = parentNode;

	parentNode->parent = rightChild;
	rightChild->left = parentNode;

	rightChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;

	return rightChild;
}

AvlIntervalTreeNode::ptr AvlIntervalTree::rotateRight(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& leftChild)
{
	parentNode->left = leftChild->right;
	if (leftChild->right)
		leftChild->right->parent = parentNode;

	parentNode->parent = leftChild;
	leftChild->right = parentNode;

	leftChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;

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
	leftChild->right = leftChildOfNodeZ;
	if (leftChildOfNodeZ)
		leftChildOfNodeZ->parent = leftChild;

	parentNode->parent = nodeZ;
	parentNode->left = rightChildOfNodeZ;
	if (rightChildOfNodeZ)
		rightChildOfNodeZ->parent = parentNode;


	if (nodeZ->balancingFactor == AvlIntervalTreeNode::BalancingFactor::Balanced)
	{
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
		leftChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	}
	else if (nodeZ->balancingFactor == AvlIntervalTreeNode::BalancingFactor::LeftHeavy)
	{
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
		leftChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	}
	else
	{
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
		leftChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
	}
	nodeZ->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
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

	if (nodeZ->balancingFactor == AvlIntervalTreeNode::BalancingFactor::Balanced)
	{
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
		rightChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	}
	else if (nodeZ->balancingFactor == AvlIntervalTreeNode::BalancingFactor::LeftHeavy)
	{
		rightChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	}
	else
	{
		parentNode->balancingFactor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
		rightChild->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
	}
	nodeZ->balancingFactor = AvlIntervalTreeNode::BalancingFactor::Balanced;
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
