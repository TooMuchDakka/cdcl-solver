#ifndef AVL_INTERVAL_TREE_NODE_HPP
#define AVL_INTERVAL_TREE_NODE_HPP

#include <memory>
#include <vector>

#include "dimacs/problemDefinition.hpp"

namespace avl {
	class AvlIntervalTreeNode {
	public:
		using ptr = std::shared_ptr<AvlIntervalTreeNode>;

		enum BalancingFactor
		{
			BALANCED = 0,
			LEFT_HEAVY = -1,
			RIGHT_HEAVY = 1
		};

		long key; // Literal mid point
		struct LiteralBoundsAndClausePair
		{
			long literalBound;
			std::size_t idxOfReferencedClauseInFormula;
			bool markedAsDeleted;

			explicit LiteralBoundsAndClausePair(long literalBound, std::size_t idxOfReferencedClauseInFormula)
				: literalBound(literalBound), idxOfReferencedClauseInFormula(idxOfReferencedClauseInFormula), markedAsDeleted(false) {}
		};
		std::vector<LiteralBoundsAndClausePair> lowerBoundsSortedAscending;
		std::vector<LiteralBoundsAndClausePair> upperBoundsSortedDescending;
		BalancingFactor internalAvlBalancingFactor;

		AvlIntervalTreeNode::ptr left;
		AvlIntervalTreeNode::ptr right;
		AvlIntervalTreeNode::ptr parent;

		explicit AvlIntervalTreeNode(const long key): key(key), internalAvlBalancingFactor(AvlIntervalTreeNode::BalancingFactor::BALANCED) {}

		friend BalancingFactor& operator++(BalancingFactor& factor);
		friend BalancingFactor& operator--(BalancingFactor& factor);

		void insertLowerBound(const LiteralBoundsAndClausePair& lowerBoundsAndReferencedClauseData);
		void insertUpperBound(const LiteralBoundsAndClausePair& upperBoundsAndReferencedClauseData);

		enum ClauseRemovalResult
		{
			Removed,
			NotFound,
			ValidationError
		};
		[[nodiscard]] ClauseRemovalResult removeIntersectedClause(std::size_t clauseIdx, const dimacs::ProblemDefinition::Clause::LiteralBounds& expectedLiteralBoundsOfClause);
		[[nodiscard]] bool doesNodeContainMatchingLiteralBoundsForClause(std::size_t clauseIdx, const dimacs::ProblemDefinition::Clause::LiteralBounds& expectedLiteralBoundsOfClause) const;

		[[nodiscard]] std::vector<std::size_t> getIntersectedClauseIndicesMovingFromSmallestLowerBoundToMidPoint(long intersectingLiteral) const;
		[[nodiscard]] std::vector<std::size_t> getIntersectedClauseIndicesMovingLargestUpperBoundToMidPoint(long intersectingLiteral) const;
 
		[[nodiscard]] std::optional<long> getLargestUpperBound() const;
		[[nodiscard]] std::optional<long> getSmallestLowerBound() const;
		[[nodiscard]] bool doesClauseIntersect(const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds) const;
		[[nodiscard]] bool isKeyContainedInInterval(const dimacs::ProblemDefinition::Clause::LiteralBounds& intervalBounds) const;
		[[nodiscard]] bool doesNodeStoreAnyInterval() const;
		[[nodiscard]] static long determineLiteralBoundsMidPoint(const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds);
		static void substituteNodeButKeepKey(const AvlIntervalTreeNode& toBeReplacedNode, const AvlIntervalTreeNode::ptr& substituteForNode);

	protected:
		[[nodiscard]] std::vector<LiteralBoundsAndClausePair>::const_iterator findLowerBoundOfClause(std::size_t idxOfClause, long lowerBoundOfClause) const;
		[[nodiscard]] std::vector<LiteralBoundsAndClausePair>::const_iterator findUpperBoundOfClause(std::size_t idxOfClause, long upperBoundOfClause) const;
		[[nodiscard]] static std::vector<LiteralBoundsAndClausePair>::const_iterator findPositionOfClauseInMatchingBounds(std::size_t idxOfClause, const std::vector<LiteralBoundsAndClausePair>::const_iterator& startPositionWithMatchingBound, const std::vector<LiteralBoundsAndClausePair>::const_iterator& lowerBoundOfSearchSpace, const std::vector<LiteralBoundsAndClausePair>::const_iterator& upperBoundOfSearchSpace);
	};
}

#endif