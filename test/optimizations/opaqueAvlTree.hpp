#ifndef OPAQUE_AVL_TREE_HPP
#define OPAQUE_AVL_TREE_HPP

#include <optimizations/avlTree.hpp>

namespace avl
{
	class OpaqueAvlTree : public AvlTree
	{
	public:
		[[nodiscard]] AvlTreeNode::ptr getRootNode() const { return root; }
	};
}

#endif
