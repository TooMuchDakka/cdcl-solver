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
		const std::size_t midPointIdx = (currLargestLowerBoundIdx + currSmallestLowerBoundIdx) / 2;
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

	if (currLargestLowerBoundIdx == lowerBoundsSortedAscending.size())
		lowerBoundsSortedAscending.push_back(lowerBoundsAndReferencedClauseData);
	else
		lowerBoundsSortedAscending.insert(std::next(lowerBoundsSortedAscending.cbegin(), currLargestLowerBoundIdx), lowerBoundsAndReferencedClauseData);
}

void AvlIntervalTreeNode::insertUpperBound(const LiteralBoundsAndClausePair& upperBoundsAndReferencedClauseData)
{
	if (upperBoundsSortedDescending.empty() || upperBoundsAndReferencedClauseData.literalBound <= upperBoundsSortedDescending.back().literalBound)
	{
		upperBoundsSortedDescending.emplace_back(upperBoundsAndReferencedClauseData);
		return;
	}

	if (upperBoundsAndReferencedClauseData.literalBound >= upperBoundsSortedDescending.front().literalBound)
	{
		upperBoundsSortedDescending.insert(upperBoundsSortedDescending.cbegin(), upperBoundsAndReferencedClauseData);
		return;
	}

	std::size_t currLargestUpperBoundIdx = 0;
	std::size_t currSmallestUpperBoundIdx = upperBoundsSortedDescending.size() - 1;

	while (currSmallestUpperBoundIdx >= currLargestUpperBoundIdx)
	{
		const std::size_t midPointIdx = (currSmallestUpperBoundIdx + currLargestUpperBoundIdx) / 2;
		const long midPointLowerBound = upperBoundsSortedDescending.at(midPointIdx).literalBound;
		if (upperBoundsAndReferencedClauseData.literalBound < midPointLowerBound)
		{
			currLargestUpperBoundIdx = midPointIdx + 1;
		}
		else
		{
			currSmallestUpperBoundIdx = midPointIdx - 1;
		}
	}

	const std::size_t insertPosition = currLargestUpperBoundIdx ? currLargestUpperBoundIdx - 1 : 0;
	if (insertPosition == upperBoundsSortedDescending.size())
		upperBoundsSortedDescending.push_back(upperBoundsAndReferencedClauseData);
	else
		upperBoundsSortedDescending.insert(std::next(upperBoundsSortedDescending.cbegin(), currLargestUpperBoundIdx), upperBoundsAndReferencedClauseData);
}

// TODO: Does not seem to work correctly for now, see configured example .cnf file (and use for debugging)
AvlIntervalTreeNode::ClauseRemovalResult AvlIntervalTreeNode::removeIntersectedClause(std::size_t clauseIdx, const dimacs::ProblemDefinition::Clause::LiteralBounds& expectedLiteralBoundsOfClause)
{
	const auto& matchingLowerBoundForClause = findLowerBoundOfClause(clauseIdx, expectedLiteralBoundsOfClause.smallestLiteral);
	const auto& matchingUpperBoundForClause = findUpperBoundOfClause(clauseIdx, expectedLiteralBoundsOfClause.largestLiteral);

	if (matchingLowerBoundForClause == lowerBoundsSortedAscending.cend() || matchingUpperBoundForClause == upperBoundsSortedDescending.cend())
		return ClauseRemovalResult::NotFound;

	if (matchingLowerBoundForClause->literalBound != expectedLiteralBoundsOfClause.smallestLiteral || matchingUpperBoundForClause->literalBound != expectedLiteralBoundsOfClause.largestLiteral)
		return ClauseRemovalResult::ValidationError;

	lowerBoundsSortedAscending.erase(matchingLowerBoundForClause);
	upperBoundsSortedDescending.erase(matchingUpperBoundForClause);
	return ClauseRemovalResult::Removed;
}

bool AvlIntervalTreeNode::doesNodeContainMatchingLiteralBoundsForClause(std::size_t clauseIdx, const dimacs::ProblemDefinition::Clause::LiteralBounds& expectedLiteralBoundsOfClause) const
{
	return findLowerBoundOfClause(clauseIdx, expectedLiteralBoundsOfClause.smallestLiteral) != lowerBoundsSortedAscending.cend() && findUpperBoundOfClause(clauseIdx, expectedLiteralBoundsOfClause.largestLiteral) != upperBoundsSortedDescending.cend();
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
		return doLiteralBoundsLieInCoveredRangeOfNode && literalBounds.smallestLiteral <= key && key <= literalBounds.largestLiteral;
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

bool AvlIntervalTreeNode::doesContainLowerBound(long lowerBound) const
{
	if (lowerBoundsSortedAscending.empty())
		return false;

	if (lowerBoundsSortedAscending.size() == 1)
		return lowerBoundsSortedAscending.front().literalBound == lowerBound;

	std::size_t smallestLowerBoundIdx = 0;
	std::size_t largestLowerBoundIdx = lowerBoundsSortedAscending.size() - 1;

	while (smallestLowerBoundIdx <= largestLowerBoundIdx)
	{
		std::size_t midPointIdx = (largestLowerBoundIdx + smallestLowerBoundIdx) / 2;
		const long lowerBoundAtMidPoint = lowerBoundsSortedAscending.at(midPointIdx).literalBound;
		if (lowerBound == lowerBoundAtMidPoint)
			return true;

		if (!midPointIdx)
			return false;

		if (lowerBound < lowerBoundAtMidPoint)
			largestLowerBoundIdx = midPointIdx - 1;
		else
			smallestLowerBoundIdx = midPointIdx + 1;
	}
	return false;
}

bool AvlIntervalTreeNode::doesContainUpperBound(long upperBound) const
{
	if (upperBoundsSortedDescending.empty())
		return false;

	if (upperBoundsSortedDescending.size() == 1)
		return upperBoundsSortedDescending.front().literalBound == upperBound;

	std::size_t largestUpperBoundIdx = 0;
	std::size_t smallestUpperBoundIdx = upperBoundsSortedDescending.size() - 1;
	while (largestUpperBoundIdx <= smallestUpperBoundIdx)
	{
		std::size_t midPointIdx = (largestUpperBoundIdx + smallestUpperBoundIdx) / 2;
		const long upperBoundAtMidPoint = upperBoundsSortedDescending.at(midPointIdx).literalBound;
		if (upperBound == upperBoundAtMidPoint)
			return true;

		if (!midPointIdx)
			return false;

		if (upperBound > upperBoundAtMidPoint)
			smallestUpperBoundIdx = midPointIdx - 1;
		else
			largestUpperBoundIdx = midPointIdx + 1;
	}
	return false;
}


std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findLowerBoundOfClause(std::size_t idxOfClause, long lowerBoundOfClause) const
{
	return findBoundOfClause(idxOfClause, lowerBoundOfClause, lowerBoundsSortedAscending);
}

std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findUpperBoundOfClause(std::size_t idxOfClause, long upperBoundOfClause) const
{
	return findBoundOfClause(idxOfClause, upperBoundOfClause, upperBoundsSortedDescending);
}

// TODO: Perform binary search for matching element
std::vector<AvlIntervalTreeNode::LiteralBoundsAndClausePair>::const_iterator AvlIntervalTreeNode::findBoundOfClause(std::size_t idxOfClause, const long literalBound, const std::vector<LiteralBoundsAndClausePair>& vectorToSearchThrough)
{
	return std::find_if(
		vectorToSearchThrough.cbegin(), 
		vectorToSearchThrough.cend(), 
		[idxOfClause, literalBound](const LiteralBoundsAndClausePair& storedLiteralBoundAndClauseData)
		{
			return storedLiteralBoundAndClauseData.idxOfReferencedClauseInFormula == idxOfClause && storedLiteralBoundAndClauseData.literalBound == literalBound;
		}
	);
}