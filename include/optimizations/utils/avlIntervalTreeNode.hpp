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

			/// The index of the first bound larger than the given literal (if sorted ascendingly) otherwise, the bound smaller than the given literal.
			/// @param literalBounds The searched through literal bounds
			/// @param literalBoundsSortOrder The sort order of the literals in the container
			/// @param literalBound The literal for which overlaps shall be determined
			///	@note The return value of 0 only indicates no overlap for a container with more than one element.
			/// @return 
			/// * No bound of the container overlapped the provided literal => 0
			/// * All bounds overlapped => container.size()
			/// * Else => [1, container.size())
			[[nodiscard]] static std::optional<std::size_t> getStopIndexForClausesOverlappingLiteral(const std::vector<long>& literalBounds, LiteralBoundsSortOrder literalBoundsSortOrder, long literalBound);
			[[nodiscard]] std::vector<std::size_t> getIndicesOfClausesOverlappingLiteralBound(long literalBound) const;

			struct ExtractedClauseBoundAndIndex
			{
				std::size_t index;
				long bound;
				explicit  ExtractedClauseBoundAndIndex(std::size_t index, long bound)
					: index(index), bound(bound) {}
			};
			[[nodiscard]] std::vector<ExtractedClauseBoundAndIndex> removeClausesOverlappingLiteralBound(long literal);
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

		struct ClauseBounds
		{
			long lowerBound;
			long upperBound;

			explicit ClauseBounds(long lowerBound, long upperBound)
				: lowerBound(lowerBound), upperBound(upperBound) {}
		};

		[[nodiscard]] std::unordered_map<std::size_t, ClauseBounds> removeClauseBoundsOverlappingLiteral(long literal);

	};
}

#endif