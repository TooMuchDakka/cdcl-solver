#include "optimizations/utils/avlIntervalTree.hpp"

using namespace avl;

std::unordered_set<std::size_t> AvlIntervalTree::determineIndicesOfClausesContainingLiteral(long literal) const
{
	if (!literal)
		return {};

	std::unordered_set<std::size_t> aggregateOfOVerlappingClauseIndices;

	AvlIntervalTreeNode::ptr currNode = avlTreeRoot;
	bool errorDuringRecordingOfOverlappingClauses = false;
	while (currNode && !errorDuringRecordingOfOverlappingClauses)
	{
		if (literal <= currNode->intervalMidPoint)
		{
			errorDuringRecordingOfOverlappingClauses = !recordClausesContainingLiteral(*formula, literal, currNode->overlappingIntervalsLowerBoundsData, aggregateOfOVerlappingClauseIndices);
			currNode = literal == currNode->intervalMidPoint ? nullptr : currNode->left;
		}
		else
		{
			errorDuringRecordingOfOverlappingClauses = !recordClausesContainingLiteral(*formula, literal, currNode->overlappingIntervalsUpperBoundsData, aggregateOfOVerlappingClauseIndices);
			currNode = literal == currNode->intervalMidPoint ? nullptr : currNode->right;
		}
	}
	return errorDuringRecordingOfOverlappingClauses ? std::unordered_set<std::size_t>() : aggregateOfOVerlappingClauseIndices;
}

bool AvlIntervalTree::insertClause(std::size_t clauseIndex, const dimacs::ProblemDefinition::Clause& clause)
{
	if (clause.literals.empty())
		return false;

	const long clauseLiteralsMidpoint = clause.determineLiteralsMidpoint();
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
		if (clause.getSmallestLiteralOfClause().value() <= traversalNode->intervalMidPoint && traversalNode->intervalMidPoint <= clause.getLargestLiteralOfClause().value())
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

AvlIntervalTree::ptr AvlIntervalTree::createFromCnfFormula(const dimacs::ProblemDefinition::ptr& cnfFormula)
{
	const std::size_t numClausesInFormula = cnfFormula->getNumClausesAfterOptimizations();
	auto avlIntervalTreeInstance = std::make_shared<AvlIntervalTree>(cnfFormula);

	std::vector<std::size_t> clauseIndicesOrderedByClauseMidpoint(numClausesInFormula, 0);
	for (std::size_t i = 0; i < numClausesInFormula; ++i)
		clauseIndicesOrderedByClauseMidpoint[i] = i;

	std::sort(
		clauseIndicesOrderedByClauseMidpoint.begin(),
		clauseIndicesOrderedByClauseMidpoint.end(),
		[&](const std::size_t lClauseIndex, const std::size_t rClauseIndex)
		{
			const dimacs::ProblemDefinition::Clause* lClause = cnfFormula->getClauseByIndexInFormula(lClauseIndex);
			const dimacs::ProblemDefinition::Clause* rClause = cnfFormula->getClauseByIndexInFormula(rClauseIndex);

			const long lClauseLiteralsMidpoint = lClause ? lClause->determineLiteralsMidpoint() : LONG_MAX;
			const long rClauseLiteralsMidpoint = rClause ? rClause->determineLiteralsMidpoint() : LONG_MAX;
			return lClauseLiteralsMidpoint == rClauseLiteralsMidpoint ? lClauseIndex < rClauseIndex : lClauseLiteralsMidpoint < rClauseLiteralsMidpoint;
		});
	if (!avlIntervalTreeInstance->performDividingInsertOfClausesSortedAccordingToLiteralsMidpoint(clauseIndicesOrderedByClauseMidpoint))
		return nullptr;
	return avlIntervalTreeInstance;
}


// BEGIN NON-PUBLIC INTERFACE IMPLEMENTATION
bool AvlIntervalTree::recordClausesContainingLiteral(const dimacs::ProblemDefinition& formula, long literal, const AvlIntervalTreeNode::ClauseBoundsAndIndices& clauseBoundsAndIndices, std::unordered_set<std::size_t>& aggregatorOfClauseIndicesContainingLiteral)
{
	const std::vector<std::size_t>& overlappingClauseIndices = clauseBoundsAndIndices.getIndicesOfClausesOverlappingLiteralBound(literal);
	for (const std::size_t clauseIndex : overlappingClauseIndices)
	{
		if (const dimacs::ProblemDefinition::Clause* referenceClause = formula.getClauseByIndexInFormula(clauseIndex); referenceClause && referenceClause->containsLiteral(literal))
			aggregatorOfClauseIndicesContainingLiteral.emplace(clauseIndex);
			
	}
	return true;
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
	moveIntervalsOverlappingParentFromChildToParent(*parentNode, *rightChild);
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
	moveIntervalsOverlappingParentFromChildToParent(*parentNode, *leftChild);
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
	moveIntervalsOverlappingParentFromChildToParent(*leftChild, *nodeZ);
	if (leftChildOfNodeZ)
	{
		leftChildOfNodeZ->parent = leftChild;
		moveIntervalsOverlappingParentFromChildToParent(*leftChildOfNodeZ, *parentNode);
	}

	parentNode->parent = nodeZ;
	parentNode->left = rightChildOfNodeZ;
	moveIntervalsOverlappingParentFromChildToParent(*parentNode, *nodeZ);
	if (rightChildOfNodeZ)
	{
		rightChildOfNodeZ->parent = parentNode;
		moveIntervalsOverlappingParentFromChildToParent(*rightChildOfNodeZ, *parentNode);
	}

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
	moveIntervalsOverlappingParentFromChildToParent(*rightChild, *nodeZ);
	if (rightChildOfNodeZ)
	{
		rightChildOfNodeZ->parent = rightChild;
		moveIntervalsOverlappingParentFromChildToParent(*rightChildOfNodeZ, *parentNode);
	}

	parentNode->right = leftChildOfNodeZ;
	parentNode->parent = nodeZ;

	moveIntervalsOverlappingParentFromChildToParent(*parentNode, *nodeZ);
	if (leftChildOfNodeZ)
	{
		leftChildOfNodeZ->parent = parentNode;
		moveIntervalsOverlappingParentFromChildToParent(*leftChildOfNodeZ, *parentNode);
	}

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

bool AvlIntervalTree::performDividingInsertOfClausesSortedAccordingToLiteralsMidpoint(const std::vector<std::size_t>& sortedClauseIndices)
{
	const std::size_t virtualClauseContainerPadding = sortedClauseIndices.size() % 2 == 0;
	const std::size_t initialMidpointIndex = std::round((sortedClauseIndices.size() + virtualClauseContainerPadding) / 2);
	if (!tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, initialMidpointIndex))
		return false;

	std::size_t lHalfMidpointIdx = std::round(initialMidpointIndex / 2);
	std::size_t rHalfMidpointIdx = initialMidpointIndex + lHalfMidpointIdx;

	bool creationSuccessful = true;
	switch (sortedClauseIndices.size())
	{
		case 1:
			break;
		case 2:
			return tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, 0);
		case 3:
			return tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, 0)
				&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, 2);
		default:
			creationSuccessful = tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, lHalfMidpointIdx)
				&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, rHalfMidpointIdx);
			break;
	}

	constexpr std::size_t offsetStepsize = 2;
	std::size_t offset = offsetStepsize;

	lHalfMidpointIdx--;
	rHalfMidpointIdx++;
	const std::size_t numIterations = lHalfMidpointIdx;
	for (std::size_t i = 0; creationSuccessful && i < numIterations; ++i) {
		creationSuccessful &= tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, lHalfMidpointIdx)
			&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, rHalfMidpointIdx)
			&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, lHalfMidpointIdx + offset)
			&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, rHalfMidpointIdx - offset);

		lHalfMidpointIdx--;
		rHalfMidpointIdx++;
		offset += offsetStepsize;
	}

	return creationSuccessful
		&& tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, lHalfMidpointIdx)
		&& (!virtualClauseContainerPadding ? tryPerformInsertOfClauseFromSortedClauseIndexContainer(sortedClauseIndices, rHalfMidpointIdx) : true);
}

bool AvlIntervalTree::tryPerformInsertOfClauseFromSortedClauseIndexContainer(const std::vector<std::size_t>& sortedClauseIndices, std::size_t indexInSortedContainer)
{
	const std::size_t clauseIndex = sortedClauseIndices.at(indexInSortedContainer);
	const dimacs::ProblemDefinition::Clause* accessedClauseForIndex = formula->getClauseByIndexInFormula(clauseIndex);
	if (!accessedClauseForIndex)
		return false;

	return insertClause(clauseIndex, *accessedClauseForIndex);
}


// TODO: Remove empty nodes
void AvlIntervalTree::moveIntervalsOverlappingParentFromChildToParent(AvlIntervalTreeNode& child, AvlIntervalTreeNode& parent)
{
	const std::unordered_map<std::size_t, AvlIntervalTreeNode::ClauseBounds>& overlappingClausesFromChild = child.removeClauseBoundsOverlappingLiteral(parent.intervalMidPoint);
	for (const auto& [clauseIndex, clauseBounds] : overlappingClausesFromChild)
	{
		parent.overlappingIntervalsLowerBoundsData.insertClause(clauseIndex, clauseBounds.lowerBound);
		parent.overlappingIntervalsUpperBoundsData.insertClause(clauseIndex, clauseBounds.upperBound);
	}
}
