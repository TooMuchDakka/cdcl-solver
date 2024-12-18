#include <unordered_map>
#include <unordered_set>
#include <optimizations/utils/avlIntervalTreeNode.hpp>
#include <optimizations/utils/binarySearchUtils.hpp>

using namespace avl;

bool AvlIntervalTreeNode::ClauseBoundsAndIndices::insertClause(std::size_t clauseIndex, long literalBound)
{
	if (literalBounds.empty())
	{
		clauseIndices.emplace_back(clauseIndex);
		literalBounds.emplace_back(literalBound);
		return true;
	}

	const std::size_t insertPosition = literalBoundsSortOrder == Ascending
		? bSearchUtils::bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(literalBounds, literalBound)
		: bSearchUtils::bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(literalBounds, literalBound);

	literalBounds.emplace(literalBounds.begin() + insertPosition, literalBound);
	clauseIndices.emplace(clauseIndices.begin() + insertPosition, clauseIndex);
	return true;
}

std::optional<std::size_t> AvlIntervalTreeNode::ClauseBoundsAndIndices::getClauseIndex(std::size_t accessKey) const
{
	return accessKey < clauseIndices.size() ? std::make_optional(clauseIndices[accessKey]) : std::nullopt;
}

std::optional<long> AvlIntervalTreeNode::ClauseBoundsAndIndices::getLiteralBound(std::size_t accessKey) const
{
	return accessKey < literalBounds.size() ? std::make_optional(literalBounds[accessKey]) : std::nullopt;
}


std::optional<std::size_t> AvlIntervalTreeNode::ClauseBoundsAndIndices::getStopIndexForClausesOverlappingLiteral(const std::vector<long>& literalBounds, LiteralBoundsSortOrder literalBoundsSortOrder, long literal)
{
	if (!literal || literalBounds.empty())
		return std::nullopt;

	const std::size_t numElementsToCheck = literalBounds.size();
	std::size_t currIdx = numElementsToCheck - 1;
	if (literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending)
	{
		if (literal >= literalBounds.back())
			return currIdx;
		if (literal < literalBounds.front())
			return std::nullopt;

		return bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(literalBounds, literal);
	}

	if (literal > literalBounds.front())
		return std::nullopt;
	if (literal <= literalBounds.back())
		return currIdx;
	return bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(literalBounds, literal);
}

std::vector<std::size_t> AvlIntervalTreeNode::ClauseBoundsAndIndices::getIndicesOfClausesOverlappingLiteralBound(long literalBound) const
{
	const std::optional<std::size_t> searchStopIndex = getStopIndexForClausesOverlappingLiteral(literalBounds, literalBoundsSortOrder, literalBound);
	const std::size_t numOverlappingIntervalsStoredInCurrentNode = searchStopIndex.value_or(SIZE_MAX) + 1;
	return { clauseIndices.cbegin(), std::next(clauseIndices.cbegin(), numOverlappingIntervalsStoredInCurrentNode) };
}

std::vector<AvlIntervalTreeNode::ClauseBoundsAndIndices::ExtractedClauseBoundAndIndex> AvlIntervalTreeNode::ClauseBoundsAndIndices::removeClausesOverlappingLiteralBound(long literalBound)
{
	std::vector<ExtractedClauseBoundAndIndex> extractedClauseBoundsAndIndices;
	if (literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending)
	{
		if (literalBound < literalBounds.front())
			return {};

		for (std::size_t i = 0; i < literalBounds.size() && literalBounds[i] <= literalBound; ++i)
			extractedClauseBoundsAndIndices.emplace_back(clauseIndices[i], literalBounds[i]);
	}
	else
	{
		if (literalBound > literalBounds.front())
			return {};

		for (std::size_t i = 0; i < literalBounds.size() && literalBounds[i] >= literalBound; ++i)
			extractedClauseBoundsAndIndices.emplace_back(clauseIndices[i], literalBounds[i]);
	}

	if (extractedClauseBoundsAndIndices.empty())
		return {};

	const std::size_t numElementsToRemove = extractedClauseBoundsAndIndices.size() - 1;
	literalBounds.erase(literalBounds.begin(), std::next(literalBounds.begin() + numElementsToRemove));
	clauseIndices.erase(clauseIndices.begin(), std::next(clauseIndices.begin() + numElementsToRemove));
	return extractedClauseBoundsAndIndices;
}

std::unordered_map<std::size_t, AvlIntervalTreeNode::ClauseBounds> AvlIntervalTreeNode::removeClauseBoundsOverlappingLiteral(long literal)
{
	if (!literal || overlappingIntervalsLowerBoundsData.isEmpty())
		return {};

	std::unordered_map<std::size_t, ClauseBounds> removedClauseBounds;
	const bool targettingLowerBounds = literal < intervalMidPoint;
	if (targettingLowerBounds)
	{
		for (const ClauseBoundsAndIndices::ExtractedClauseBoundAndIndex extractedLowerBoundData : overlappingIntervalsLowerBoundsData.removeClausesOverlappingLiteralBound(literal))
			removedClauseBounds.emplace(extractedLowerBoundData.index, ClauseBounds(extractedLowerBoundData.bound, 0));
	}
	else
	{
		for (const ClauseBoundsAndIndices::ExtractedClauseBoundAndIndex extractedUpperBoundData : overlappingIntervalsUpperBoundsData.removeClausesOverlappingLiteralBound(literal))
			removedClauseBounds.emplace(extractedUpperBoundData.index, ClauseBounds(0,extractedUpperBoundData.bound));
	}

	const std::size_t numEntriesToRemoveInOtherNodeHalf = removedClauseBounds.size();
	std::vector<std::size_t> indicesOfMatchingClauseInOtherHalf(numEntriesToRemoveInOtherNodeHalf, 0);
	if (!numEntriesToRemoveInOtherNodeHalf)
		return {};

	std::unordered_set<std::size_t> toBeRemovedElementIndicesInOtherHalf;
	toBeRemovedElementIndicesInOtherHalf.reserve(numEntriesToRemoveInOtherNodeHalf);

	AvlIntervalTreeNode::ClauseBoundsAndIndices& clauseBoundsAndIndicesOfOtherHalf = targettingLowerBounds
		? overlappingIntervalsUpperBoundsData
		: overlappingIntervalsLowerBoundsData;

	std::vector<long>& toBeModifiedLiteralBounds = clauseBoundsAndIndicesOfOtherHalf.literalBounds;
	std::vector<std::size_t>& toBeModifiedClauseIndices = clauseBoundsAndIndicesOfOtherHalf.clauseIndices;

	const std::size_t numElementsToCheck = toBeModifiedLiteralBounds.size();
	for (std::size_t i = 0; i < numElementsToCheck; ++i)
	{
		const std::size_t clauseIndex = toBeModifiedClauseIndices.at(i);
		if (!removedClauseBounds.count(clauseIndex))
			continue;

		toBeRemovedElementIndicesInOtherHalf.emplace(i);
		if (targettingLowerBounds)
			removedClauseBounds.at(clauseIndex).upperBound = toBeModifiedLiteralBounds.at(i);
		else
			removedClauseBounds.at(clauseIndex).lowerBound = toBeModifiedLiteralBounds.at(i);
	}

	toBeModifiedLiteralBounds.erase(
		removeIndexesIf(
			toBeModifiedLiteralBounds.begin(),
			toBeModifiedLiteralBounds.end(),
			[&toBeRemovedElementIndicesInOtherHalf](const std::size_t indexInContainer)
			{
				return toBeRemovedElementIndicesInOtherHalf.count(indexInContainer);
			}),
		toBeModifiedLiteralBounds.end()
	);

	toBeModifiedClauseIndices.erase(
		removeIndexesIf(
			toBeModifiedClauseIndices.begin(),
			toBeModifiedClauseIndices.end(),
			[&toBeRemovedElementIndicesInOtherHalf](const std::size_t indexInContainer)
			{
				return toBeRemovedElementIndicesInOtherHalf.count(indexInContainer);
			}),
		toBeModifiedClauseIndices.end()
	);
	return removedClauseBounds;
}

long AvlIntervalTreeNode::getSmallestLiteralBoundOfOverlappedClauses() const
{
	return overlappingIntervalsLowerBoundsData.literalBounds.front();
}

long AvlIntervalTreeNode::getLargestLiteralBoundOfOverlappedClauses() const
{
	return overlappingIntervalsUpperBoundsData.literalBounds.front();
}

AvlIntervalTreeNode::BalancingFactor& avl::operator++(AvlIntervalTreeNode::BalancingFactor& factor)
{
	switch (factor)
	{
	case AvlIntervalTreeNode::BalancingFactor::Balanced:
		factor = AvlIntervalTreeNode::BalancingFactor::RightHeavy;
		break;
	case AvlIntervalTreeNode::BalancingFactor::LeftHeavy:
		factor = AvlIntervalTreeNode::BalancingFactor::Balanced;
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
	case AvlIntervalTreeNode::BalancingFactor::Balanced:
		factor = AvlIntervalTreeNode::BalancingFactor::LeftHeavy;
		break;
	case AvlIntervalTreeNode::BalancingFactor::RightHeavy:
		factor = AvlIntervalTreeNode::BalancingFactor::Balanced;
		break;
	default:
		break;
	}
	return factor;
}
