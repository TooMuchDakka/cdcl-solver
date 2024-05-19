#ifndef AVL_TREE_TESTS_FIXTURE_HPP
#define AVL_TREE_TESTS_FIXTURE_HPP

#include <algorithm>

#include "opaqueAvlTree.hpp"
#include <gtest/gtest.h>

class AvlTreeTestsFixture : public ::testing::Test
{
protected:
	void SetUp() override {}

	[[nodiscard]] static bool areVectorsEqual(const std::vector<long>& expected, const std::vector<long>& actual)
	{
		if (expected.size() == actual.size())
		{
			const auto& missmatchPair = std::mismatch(expected.cbegin(), expected.cend(), actual.cbegin());
			return missmatchPair.first == expected.cend() && missmatchPair.second == actual.cend();
		}
		return false;
	}

	static void assertThatAvlTreeNodesMatch(const avl::AvlTree::AvlTreeNode::ptr& expected, const avl::AvlTree::AvlTreeNode::ptr& actual)
	{
		ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodeKeysMatch(expected, actual));
		if (!expected)
			return;

		ASSERT_EQ(expected->getReferenceCount(), actual->getReferenceCount()) << "Expected reference count was " << std::to_string(expected->getReferenceCount()) + " but was actually " << std::to_string(actual->getReferenceCount()) << " in node with key " << expected->key;
		ASSERT_EQ(expected->balancingFactor, actual->balancingFactor) << "Expected balancing factor " << std::to_string(expected->balancingFactor) << " but was actually " << std::to_string(actual->balancingFactor) << " in node with key " << expected->key;
		ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodeKeysMatch(expected->parent, actual->parent)) << "parent node missmatch in node with key " << expected->key;
		ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expected->left, actual->left)) << "Left child missmatch in node with key " << std::to_string(expected->key);
		ASSERT_NO_FATAL_FAILURE(assertThatAvlTreeNodesMatch(expected->right, actual->right)) << "Right child missmatch in node with key " << std::to_string(expected->key);
	}

	static void assertThatAvlTreeNodeKeysMatch(const avl::AvlTree::AvlTreeNode::ptr& expected, const avl::AvlTree::AvlTreeNode::ptr& actual)
	{
		if (!expected)
			ASSERT_FALSE(actual) << "Expected tree node to not be set";
		else
			ASSERT_TRUE(expected->key == actual->key) << "Expected key was: " << std::to_string(expected->key) << " but was actually " << std::to_string(actual->key);
	}

	static void assertThatInsertOfLiteralsDoesNotFail(avl::AvlTree& avlTree, const std::vector<long>& literals)
	{
		for (const long literal : literals)
			ASSERT_TRUE(avlTree.insert(literal));
	}
};

#endif