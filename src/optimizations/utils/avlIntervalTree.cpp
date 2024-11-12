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
	return false;
}

// BEGIN NON-PUBLIC INTERFACE IMPLEMENTATION
bool AvlIntervalTree::AvlIntervalTreeNode::ClauseBoundsAndIndices::insertClause(std::size_t clauseIndex, long literalBound)
{
	literalBounds.emplace_back(literalBound);
	clauseIndices.emplace_back(clauseIndex);
	if (sortedAccessKeys.empty())
	{
		sortedAccessKeys.emplace_back(0);
		return true;
	}

	const std::size_t insertPosition = literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending
		? bSearchUtils::bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(literalBounds, sortedAccessKeys, literalBound)
		: bSearchUtils::bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(literalBounds, sortedAccessKeys, literalBound);
	
	sortedAccessKeys.emplace(sortedAccessKeys.begin() + insertPosition, literalBound);
	return true;
}

std::optional<std::size_t> AvlIntervalTree::AvlIntervalTreeNode::ClauseBoundsAndIndices::getClauseIndex(std::size_t accessKey) const
{
	return accessKey < sortedAccessKeys.size() && sortedAccessKeys[accessKey] < clauseIndices.size()
		? std::make_optional(clauseIndices[sortedAccessKeys[accessKey]])
		: std::nullopt;
}

std::optional<long> AvlIntervalTree::AvlIntervalTreeNode::ClauseBoundsAndIndices::getLiteralBound(std::size_t accessKey) const
{
	return accessKey < sortedAccessKeys.size() && sortedAccessKeys[accessKey] < literalBounds.size()
		? std::make_optional(literalBounds[sortedAccessKeys[accessKey]])
		: std::nullopt;
}

std::vector<std::size_t> AvlIntervalTree::AvlIntervalTreeNode::ClauseBoundsAndIndices::getIndicesOfClausesOverlappingLiteralBound(long literalBound) const
{
	const std::size_t searchStopIndex = literalBoundsSortOrder == LiteralBoundsSortOrder::Ascending
		? bSearchUtils::bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(literalBounds, sortedAccessKeys, literalBound)
		: bSearchUtils::bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(literalBounds, sortedAccessKeys, literalBound);

	const std::size_t numOverlappingIntervalsStoredInCurrentNode = (literalBounds.size() - searchStopIndex) + 1;

	std::vector<std::size_t> clauseIndices;
	clauseIndices.reserve(numOverlappingIntervalsStoredInCurrentNode);
	for (std::size_t i = 0; i < numOverlappingIntervalsStoredInCurrentNode; ++i)
		clauseIndices[i] = clauseIndices[sortedAccessKeys[searchStopIndex + i]];

	return clauseIndices;
}

bool AvlIntervalTree::recordClausesContainingLiteral(const dimacs::ProblemDefinition& formula, long literal, const std::vector<std::size_t>& clauseIndices, std::unordered_set<std::size_t>& aggregatorOfClauseIndicesContainingLiteral)
{
	for (const std::size_t clauseIndex : clauseIndices)
	{
		const std::optional<const dimacs::ProblemDefinition::Clause*> referenceClause = formula.getClauseByIndexInFormula(clauseIndex);
		if (!referenceClause)
			return false;

		const std::vector<long>& clauseLiterals = referenceClause.value()->literals;
		if (!bSearchUtils::bSearchInSortedContainer(clauseLiterals, std::nullopt, literal, bSearchUtils::SortOrder::Ascending))
			continue;
		aggregatorOfClauseIndicesContainingLiteral.emplace(clauseIndex);
	}
	return true;
}
