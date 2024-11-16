#ifndef AVL_INTERVAL_TREE_HPP
#define AVL_INTERVAL_TREE_HPP

#include <unordered_set>
#include <dimacs/problemDefinition.hpp>
#include <optimizations/utils/avlIntervalTreeNode.hpp>

// https://arxiv.org/ftp/arxiv/papers/1509/1509.05053.pdf
// https://stackoverflow.com/questions/11360831/about-the-branchless-binary-search
// https://devblogs.microsoft.com/oldnewthing/20230522-00/?p=108226
// https://www.cs.csub.edu/~msarr/visualizations/AVLtree.html
namespace avl {
	class AvlIntervalTree {
	public:
		using ptr = std::shared_ptr<AvlIntervalTree>;

		explicit AvlIntervalTree(dimacs::ProblemDefinition::ptr formula)
			: formula(std::move(formula)) {}

		[[nodiscard]] std::unordered_set<std::size_t> determineIndicesOfClausesContainingLiteral(long literal) const;
		[[maybe_unused]] bool insertClause(std::size_t clauseIndex, const dimacs::ProblemDefinition::Clause& clause);
	protected:
		[[maybe_unused]] static bool recordClausesContainingLiteral(const dimacs::ProblemDefinition& formula, long literal, const AvlIntervalTreeNode::ClauseBoundsAndIndices& clauseBoundsAndIndices, std::unordered_set<std::size_t>& aggregatorOfClauseIndicesContainingLiteral);
		[[nodiscard]] static long determineLiteralBoundsMidPoint(const dimacs::ProblemDefinition::Clause& clause);

		/*
		 *			P
		 *		X		Y
		 *			T1		T2
		 *
		 *
		 *			Y
		 *		P		T2
		 *	X		T1
		 */
		[[maybe_unused]] static AvlIntervalTreeNode::ptr rotateLeft(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& rightChild);

		/*
		 *			P
		 *		X		Y
		 *	T1		T2
		 *
		 *
		 *			X
		 *		T1		P
		 *			T2		Y
		 */
		[[maybe_unused]] static AvlIntervalTreeNode::ptr rotateRight(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& leftChild);

		/*
		 *			P
		 *		X		Y
		 *	t1		Z		t4
		 *		t2		t3
		 *
		 *			P
		 *		X		Z
		 *	t1		t2		Y
		 *				t3		t4
		 *
		 *				Z
		 *			P			Y
		 *		X		t2	t3		t4
		 *	t1
		 */
		[[maybe_unused]] static AvlIntervalTreeNode::ptr rotateRightLeft(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& rightChild);

		/*
		 *			P
		 *		X		Y
		 *	t1		Z		t4
		 *		t2		t3
		 *
		 *				P
		 *			Z		Y
		 *		X		t3		t4
		 *	t1		t2
		 *
		 *
		 *				Z
		 *		X			P
		 *	t1		t2	t3		Y
		 *							t4
		 *
		 */
		[[maybe_unused]] static AvlIntervalTreeNode::ptr rotateLeftRight(const AvlIntervalTreeNode::ptr& parentNode, const AvlIntervalTreeNode::ptr& leftChild);
		[[nodiscard]] static AvlIntervalTreeNode::ptr findInorderSuccessorOfNode(const AvlIntervalTreeNode::ptr& node);
		static void moveIntervalsOverlappingParentFromChildToParent(AvlIntervalTreeNode& child, AvlIntervalTreeNode& parent);

		dimacs::ProblemDefinition::ptr formula;
		AvlIntervalTreeNode::ptr avlTreeRoot;
	};
}

#endif 