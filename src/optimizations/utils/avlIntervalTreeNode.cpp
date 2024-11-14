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
	if (literalBounds.empty())
		return std::nullopt;

	std::size_t searchStopIndex = literalBounds.size() - 1;
	if (literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending)
	{
		if (literal > literalBounds.back())
			return  literalBounds.size() > 1 ? std::nullopt : std::make_optional(0);

		if (literal < literalBounds.front())
			return searchStopIndex;
	}
	else
	{
		if (literal < literalBounds.back())
			return literalBounds.size() > 1 ? std::nullopt : std::make_optional(0);

		if (literal > literalBounds.front())
			return searchStopIndex;
	}

	for (std::size_t i = 0; i < literalBounds.size(); ++i)
	{
		if (literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending
			? literalBounds.at(i) > literal
			: literalBounds.at(i) < literal)
			return i;
	}
	return searchStopIndex;
}

std::vector<std::size_t> AvlIntervalTreeNode::ClauseBoundsAndIndices::getIndicesOfClausesOverlappingLiteralBound(long literalBound) const
{
	const std::optional<std::size_t> searchStopIndex = getStopIndexForClausesOverlappingLiteral(literalBounds, literalBoundsSortOrder, literalBound);
	if (!searchStopIndex.has_value())
		return {};

	/*if ((!searchStopIndex && literalBounds.size() > 1) || searchStopIndex == literalBounds.size())
		return {};*/

	const std::size_t numOverlappingIntervalsStoredInCurrentNode = std::min(literalBounds.size(), *searchStopIndex+ 1);
	std::vector<std::size_t> indicesOfClausesOverlappingLiteral;
	indicesOfClausesOverlappingLiteral.resize(numOverlappingIntervalsStoredInCurrentNode);

	for (std::size_t i = 0; i < numOverlappingIntervalsStoredInCurrentNode; ++i)
		indicesOfClausesOverlappingLiteral[i] = clauseIndices[i];
	return indicesOfClausesOverlappingLiteral;
}

std::vector<AvlIntervalTreeNode::ClauseBoundsAndIndices::ExtractedClauseBoundAndIndex> AvlIntervalTreeNode::ClauseBoundsAndIndices::removeClausesOverlappingLiteralBound(long literalBound)
{
	const std::optional<std::size_t> searchStopIndex = getStopIndexForClausesOverlappingLiteral(literalBounds, literalBoundsSortOrder, literalBound);
	if (!searchStopIndex.has_value())
		return {};

	const std::size_t numOverlappingIntervalsStoredInCurrentNode = std::min(literalBounds.size(), *searchStopIndex + 1);
	std::vector<ExtractedClauseBoundAndIndex> extractedClauseBoundsAndIndices(numOverlappingIntervalsStoredInCurrentNode, ExtractedClauseBoundAndIndex(0,0));

	for (std::size_t i = 0; i < numOverlappingIntervalsStoredInCurrentNode; ++i)
	{
		extractedClauseBoundsAndIndices[i].index = clauseIndices.at(i);
		extractedClauseBoundsAndIndices[i].bound = literalBounds.at(i);
	}
	literalBounds.erase(literalBounds.begin(), std::next(literalBounds.begin() + numOverlappingIntervalsStoredInCurrentNode));
	return extractedClauseBoundsAndIndices;
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
