#ifndef AVL_INTERVAL_TREE_HPP
#define AVL_INTERVAL_TREE_HPP

#include <unordered_set>
#include <dimacs/problemDefinition.hpp>

// https://arxiv.org/ftp/arxiv/papers/1509/1509.05053.pdf
// https://stackoverflow.com/questions/11360831/about-the-branchless-binary-search
// https://devblogs.microsoft.com/oldnewthing/20230522-00/?p=108226
namespace avl {
	class AvlIntervalTree {
	public:
		using ptr = std::shared_ptr<AvlIntervalTree>;

		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfClausesContainingLiteral(long literal) const;
		[[maybe_unsed]] bool insertClause(std::size_t clauseIndex, const dimacs::ProblemDefinition::Clause& clause);
	protected:
		struct AvlIntervalTreeNode
		{
			using ptr = std::shared_ptr<AvlIntervalTreeNode>;
			struct ClauseBoundsAndIndices
			{
				enum LiteralBoundsSortOrder
				{
					Ascending,
					Descending
				};

				explicit ClauseBoundsAndIndices(LiteralBoundsSortOrder literalBoundsSortOrder)
					: literalBoundsSortOrder(literalBoundsSortOrder) {}

				LiteralBoundsSortOrder literalBoundsSortOrder;
				std::vector<long> literalBounds;
				std::vector<std::size_t> clauseIndices;
				std::vector<std::size_t> sortedAccessKeys;

				[[maybe_unused]] bool insertClause(std::size_t clauseIndex, long literalBound);
				[[nodiscard]] std::optional<std::size_t> getClauseIndex(std::size_t accessKey) const;
				[[nodiscard]] std::optional<long> getLiteralBound(std::size_t accessKey) const;
				[[nodiscard]] std::vector<std::size_t> getIndicesOfClausesOverlappingLiteralBound(long literalBound) const;
			};

			explicit AvlIntervalTreeNode(long intervalMidPoint, AvlIntervalTreeNode::ptr parent)
				: intervalMidPoint(intervalMidPoint), parent(std::move(parent)), left(nullptr), right(nullptr),
					overlappingIntervalsLowerBoundsData(ClauseBoundsAndIndices(ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending)),
					overlappingIntervalsUpperBoundsData(ClauseBoundsAndIndices(ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending)) {}

			long intervalMidPoint;
			AvlIntervalTreeNode::ptr parent;
			AvlIntervalTreeNode::ptr left;
			AvlIntervalTreeNode::ptr right;
			ClauseBoundsAndIndices overlappingIntervalsLowerBoundsData;
			ClauseBoundsAndIndices overlappingIntervalsUpperBoundsData;
		};
		using AvlIntervalTreeNodePointer = std::shared_ptr<AvlIntervalTreeNode>;

		[[maybe_unused]] static bool recordClausesContainingLiteral(const dimacs::ProblemDefinition& formula, long literal, const std::vector<std::size_t>& clauseIndices, std::unordered_set<std::size_t>& aggregatorOfClauseIndicesContainingLiteral);

		dimacs::ProblemDefinition::ptr formula;
		AvlIntervalTreeNode::ptr avlTreeRoot;
	};
}

#endif 