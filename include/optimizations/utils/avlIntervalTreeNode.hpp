#ifndef AVL_INTERVAL_TREE_NODE_HPP
#define AVL_INTERVAL_TREE_NODE_HPP

#include <memory>
#include <optional>
#include <vector>

namespace avl {
	class AvlIntervalTreeNode {
	public:
		using ptr = std::shared_ptr<AvlIntervalTreeNode>;

		enum BalancingFactor : char
		{
			Balanced = 0,
			LeftHeavy = -1,
			RightHeavy = 1
		};

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

			[[maybe_unused]] bool insertClause(std::size_t clauseIndex, long literalBound);
			[[nodiscard]] std::optional<std::size_t> getClauseIndex(std::size_t accessKey) const;
			[[nodiscard]] std::optional<long> getLiteralBound(std::size_t accessKey) const;
			[[nodiscard]] std::vector<std::size_t> getIndicesOfClausesOverlappingLiteralBound(long literalBound) const;
		};

		explicit AvlIntervalTreeNode(long intervalMidPoint, AvlIntervalTreeNode::ptr parent)
			: balancingFactor(BalancingFactor::Balanced), intervalMidPoint(intervalMidPoint), parent(std::move(parent)), left(nullptr), right(nullptr),
			overlappingIntervalsLowerBoundsData(ClauseBoundsAndIndices(ClauseBoundsAndIndices::LiteralBoundsSortOrder::Ascending)),
			overlappingIntervalsUpperBoundsData(ClauseBoundsAndIndices(ClauseBoundsAndIndices::LiteralBoundsSortOrder::Descending)) {}

		BalancingFactor balancingFactor;
		long intervalMidPoint;
		AvlIntervalTreeNode::ptr parent;
		AvlIntervalTreeNode::ptr left;
		AvlIntervalTreeNode::ptr right;
		ClauseBoundsAndIndices overlappingIntervalsLowerBoundsData;
		ClauseBoundsAndIndices overlappingIntervalsUpperBoundsData;

		[[nodiscard]] long getSmallestLiteralBoundOfOverlappedClauses() const;
		[[nodiscard]] long getLargestLiteralBoundOfOverlappedClauses() const;
		friend AvlIntervalTreeNode::BalancingFactor& operator++(AvlIntervalTreeNode::BalancingFactor& factor);
		friend AvlIntervalTreeNode::BalancingFactor& operator--(AvlIntervalTreeNode::BalancingFactor& factor);

	};
}

#endif