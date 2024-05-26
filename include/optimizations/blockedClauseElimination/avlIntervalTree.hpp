#ifndef AVL_INTERVAL_TREE_HPP
#define AVL_INTERVAL_TREE_HPP

#include "avlIntervalTreeNode.hpp"
#include "dimacs/problemDefinition.hpp"

namespace avl {
	class AvlIntervalTree {
	public:
		[[maybe_unused]] bool insertClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds);
		[[nodiscard]] std::vector<std::size_t> getOverlappingIntervalsForLiteral(long literal) const;
		[[maybe_unused]] bool removeClause(std::size_t clauseIdxInFormula, const dimacs::ProblemDefinition::Clause::LiteralBounds& literalBounds);

	protected:
		AvlIntervalTreeNode::ptr root;

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
		static void replaceNodeInAvlTreeStructure(const AvlIntervalTreeNode::ptr& nodeToReplace, const AvlIntervalTreeNode::ptr& replacementNode);
	};
}
#endif