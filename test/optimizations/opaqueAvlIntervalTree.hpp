#ifndef OPAQUE_AVL_INTERVAL_TREE_HPP
#define OPAQUE_AVL_INTERVAL_TREE_HPP
#include "optimizations/blockedClauseElimination/avlIntervalTree.hpp"

namespace avl {
	class OpaqueAvlIntervalTree : public AvlIntervalTree {
	public:
		[[nodiscard]] avl::AvlIntervalTreeNode::ptr getRootNode() const
		{
			return root;
		}
	};
}

#endif