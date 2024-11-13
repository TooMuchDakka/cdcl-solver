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

std::vector<std::size_t> AvlIntervalTreeNode::ClauseBoundsAndIndices::getIndicesOfClausesOverlappingLiteralBound(long literalBound) const
{
	const std::size_t searchStopIndex = literalBoundsSortOrder == Ascending
		? bSearchUtils::bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(literalBounds, literalBound)
		: bSearchUtils::bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(literalBounds, literalBound);

	const std::size_t numOverlappingIntervalsStoredInCurrentNode = literalBounds.size() - searchStopIndex;
	if (!numOverlappingIntervalsStoredInCurrentNode)
		return {};

	std::vector<std::size_t> indicesOfClausesOverlappingLiteral;
	indicesOfClausesOverlappingLiteral.resize(numOverlappingIntervalsStoredInCurrentNode);

	for (std::size_t i = 0; i < numOverlappingIntervalsStoredInCurrentNode; ++i)
		indicesOfClausesOverlappingLiteral[i] = clauseIndices[searchStopIndex + i];
	return indicesOfClausesOverlappingLiteral;
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
