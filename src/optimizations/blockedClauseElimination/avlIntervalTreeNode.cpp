#include "optimizations/blockedClauseElimination/avlIntervalTreeNode.hpp"

using namespace avl;

AvlIntervalTreeNode::BalancingFactor& avl::operator++(AvlIntervalTreeNode::BalancingFactor& factor)
{
	switch (factor)
	{
	case avl::AvlIntervalTreeNode::BalancingFactor::BALANCED:
		factor = avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY;
		break;
	case avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY:
		factor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
		break;
	default:
		break;
	}
	return factor;
}

AvlIntervalTreeNode::BalancingFactor& avl::operator--(AvlIntervalTreeNode::BalancingFactor& factor)
{
	switch (factor)
	{
	case avl::AvlIntervalTreeNode::BalancingFactor::BALANCED:
		factor = avl::AvlIntervalTreeNode::BalancingFactor::LEFT_HEAVY;
		break;
	case avl::AvlIntervalTreeNode::BalancingFactor::RIGHT_HEAVY:
		factor = avl::AvlIntervalTreeNode::BalancingFactor::BALANCED;
		break;
	default:
		break;
	}
	return factor;
}

void AvlIntervalTreeNode::insertLowerBound(const LiteralBoundsAndClausePair& lowerBoundsAndReferencedClauseData)
{
	if (lowerBoundsSortedAscending.empty() || lowerBoundsAndReferencedClauseData.literalBound >= lowerBoundsSortedAscending.back().literalBound)
	{
		lowerBoundsSortedAscending.emplace_back(lowerBoundsAndReferencedClauseData);
		return;
	}

	if (lowerBoundsAndReferencedClauseData.literalBound <= lowerBoundsSortedAscending.front().literalBound)
	{
		lowerBoundsSortedAscending.insert(lowerBoundsSortedAscending.cbegin(), lowerBoundsAndReferencedClauseData);
		return;
	}

	std::size_t currLargestLowerBoundIdx = lowerBoundsSortedAscending.size() - 1;
	std::size_t currSmallestLowerBoundIdx = 0;

	while (currSmallestLowerBoundIdx <= currLargestLowerBoundIdx)
	{
		const std::size_t midPointIdx = (currLargestLowerBoundIdx - currSmallestLowerBoundIdx) / 2;
		const long midPointLowerBound = lowerBoundsSortedAscending.at(midPointIdx).literalBound;
		if (lowerBoundsAndReferencedClauseData.literalBound < midPointLowerBound)
		{
			currLargestLowerBoundIdx = midPointIdx - 1;
		}
		else
		{
			currSmallestLowerBoundIdx = midPointIdx + 1;
		}
	}
	lowerBoundsSortedAscending.insert(std::next(lowerBoundsSortedAscending.cbegin(), currLargestLowerBoundIdx + 1), lowerBoundsAndReferencedClauseData);
}

void AvlIntervalTreeNode::insertUpperBound(const LiteralBoundsAndClausePair& upperBoundsAndReferencedClauseData)
{
	if (upperBoundsSortedDescending.empty() || upperBoundsAndReferencedClauseData.literalBound <= upperBoundsSortedDescending.back().literalBound)
	{
		upperBoundsSortedDescending.emplace_back(upperBoundsAndReferencedClauseData);
		return;
	}

	if (upperBoundsAndReferencedClauseData.literalBound >= upperBoundsSortedDescending.back().literalBound)
	{
		upperBoundsSortedDescending.insert(upperBoundsSortedDescending.cbegin(), upperBoundsAndReferencedClauseData);
		return;
	}

	std::size_t currLargestUpperBoundIdx = 0;
	std::size_t currSmallestUpperBoundIdx = upperBoundsSortedDescending.size() - 1;

	while (currLargestUpperBoundIdx >= currSmallestUpperBoundIdx)
	{
		const std::size_t midPointIdx = (currSmallestUpperBoundIdx - currLargestUpperBoundIdx) / 2;
		const long midPointLowerBound = upperBoundsSortedDescending.at(midPointIdx).literalBound;
		if (upperBoundsAndReferencedClauseData.literalBound < midPointLowerBound)
		{
			currLargestUpperBoundIdx = midPointIdx - 1;
		}
		else
		{
			currSmallestUpperBoundIdx = midPointIdx + 1;
		}
	}
	upperBoundsSortedDescending.insert(std::next(upperBoundsSortedDescending.cbegin(), currLargestUpperBoundIdx + 1), upperBoundsAndReferencedClauseData);
}

bool AvlIntervalTreeNode::removeIntersectedClause(std::size_t clauseIdx, const dimacs::ProblemDefinition::Clause::LiteralBounds& expectedLiteralBoundsOfClause)
{
	const auto& matchingLowerBoundForClause = findLowerBoundOfClause(clauseIdx);
	const auto& matchingUpperBoundForClause = findUpperBoundOfClause(clauseIdx);

	if (matchingLowerBoundForClause == lowerBoundsSortedAscending.cend() || matchingUpperBoundForClause == upperBoundsSortedDescending.cend())
		return false;

	if (matchingLowerBoundForClause->literalBound != expectedLiteralBoundsOfClause.smallestLiteral || matchingUpperBoundForClause->literalBound != expectedLiteralBoundsOfClause.largestLiteral)
		return false;

	lowerBoundsSortedAscending.erase(matchingLowerBoundForClause);
	upperBoundsSortedDescending.erase(matchingUpperBoundForClause);
	return true;
}

std::vector<std::size_t> AvlIntervalTreeNode::getIntersectedClauseIndicesMovingFromSmallestLowerBoundToMidPoint(long intersectingLiteral) const
{
	std::vector<std::size_t> intersectedClauses;
	for (auto reverseLowerBoundIterator = lowerBoundsSortedAscending.begin(); reverseLowerBoundIterator != lowerBoundsSortedAscending.end() && intersectingLiteral >= reverseLowerBoundIterator->literalBound; ++reverseLowerBoundIterator)
	{
		intersectedClauses.emplace_back(reverseLowerBoundIterator->idxOfReferencedClauseInFormula);
	}
	return intersectedClauses;
}

std::vector<std::size_t> AvlIntervalTreeNode::getIntersectedClauseIndicesMovingLargestUpperBoundToMidPoint(long intersectingLiteral) const
{
	std::vector<std::size_t> intersectedClauses;
	for (auto reverseUpperBoundIterator = upperBoundsSortedDescending.begin(); reverseUpperBoundIterator != upperBoundsSortedDescending.end() && intersectingLiteral <= reverseUpperBoundIterator->literalBound; ++reverseUpperBoundIterator)
	{
		intersectedClauses.emplace_back(reverseUpperBoundIterator->idxOfReferencedClauseInFormula);
	}
	return intersectedClauses;
}

std::optional<long> AvlIntervalTreeNode::getLargestUpperBound() const
{
	if (upperBoundsSortedDescending.empty())
		return std::nullopt;

	return  upperBoundsSortedDescending.front().literalBound;
}

std::optional<long> AvlIntervalTreeNode::getSmallestLowerBound() const
{
	if (lowerBoundsSortedAscending.empty())
		return std::nullopt;

	return lowerBoundsSortedAscending.front().literalBound;
}

bool AvlIntervalTreeNode::doesClauseIntersect(const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds) const
{
	const bool doLiteralBoundsLieInCoveredRangeOfNode = !(literalBounds.largestLiteral < getSmallestLowerBound().value_or(key) || literalBounds.smallestLiteral > getLargestUpperBound().value_or(key));
	if (literalBounds.smallestLiteral != literalBounds.largestLiteral)
		return doLiteralBoundsLieInCoveredRangeOfNode && literalBounds.smallestLiteral <= key && literalBounds.largestLiteral >= key;

	return doLiteralBoundsLieInCoveredRangeOfNode;
}

bool AvlIntervalTreeNode::doesNodeStoreAnyInterval() const
{
	return !lowerBoundsSortedAscending.empty();
}

// BEGIN NON-PUBLIC FUNCTIONALITY

// TODO: overflow handling of long
long AvlIntervalTreeNode::determineLiteralBoundsMidPoint(const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds)
{
	if (const long literalBoundsSum = literalBounds.largestLiteral + literalBounds.smallestLiteral)
	{
		return static_cast<long>(std::round(literalBoundsSum / static_cast<double>(2)));
	}
	return 0;
}

void AvlIntervalTreeNode::substituteNodeButKeepKey(const AvlIntervalTreeNode& toBeReplacedNode, const AvlIntervalTreeNode::ptr& substituteForNode)
{
	if (!substituteForNode)
		return;

	substituteForNode->parent = toBeReplacedNode.parent;

	substituteForNode->left = toBeReplacedNode.left;
	if (toBeReplacedNode.left)
		toBeReplacedNode.left->parent = substituteForNode;

	substituteForNode->right = toBeReplacedNode.right;
	if (toBeReplacedNode.right)
		toBeReplacedNode.right->parent = substituteForNode;

	substituteForNode->internalAvlBalancingFactor = toBeReplacedNode.internalAvlBalancingFactor;
}


std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findLowerBoundOfClause(std::size_t idxOfClause) const
{
	return findBoundOfClause(idxOfClause, lowerBoundsSortedAscending);
}

std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findUpperBoundOfClause(std::size_t idxOfClause) const
{
	return findBoundOfClause(idxOfClause, upperBoundsSortedDescending);
}

std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findBoundOfClause(std::size_t idxOfClause, const std::vector<LiteralBoundsAndClausePair>& vectorToSearchThrough)
{
	return std::find_if(vectorToSearchThrough.cbegin(), vectorToSearchThrough.cend(), [idxOfClause](const LiteralBoundsAndClausePair& storedLiteralBoundAndClauseData) {return storedLiteralBoundAndClauseData.idxOfReferencedClauseInFormula == idxOfClause; });
}