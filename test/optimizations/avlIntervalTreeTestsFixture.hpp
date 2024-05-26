#ifndef AVL_INTERVAL_TREE_TESTS_FIXTURE_HPP
#define AVL_INTERVAL_TREE_TESTS_FIXTURE_HPP
#include "gtest/gtest.h"
#include "optimizations/blockedClauseElimination/avlIntervalTreeNode.hpp"

namespace avlIntervalTreeTests {
	class AvlIntervalTreeTestsFixture : public ::testing::Test
	{
	protected:
		const std::vector<std::size_t> EMPTY_CLAUSE_INDICES_RESULT;

		void SetUp() override {}

		static void assertThatNodesAreEqual(const avl::AvlIntervalTreeNode::ptr& expected, const avl::AvlIntervalTreeNode::ptr& actual)
		{
			if (!expected)
			{
				ASSERT_FALSE(actual);
				return;
			}

			ASSERT_TRUE(actual);
			ASSERT_EQ(expected->key, actual->key);
			ASSERT_EQ(expected->internalAvlBalancingFactor, actual->internalAvlBalancingFactor) << "Internal balancing factor missmatch in node with key " << std::to_string(expected->key);
			ASSERT_NO_FATAL_FAILURE(assertThatLiteralBoundsAndAssociatedClausesAreEqual(expected->lowerBoundsSortedAscending, actual->lowerBoundsSortedAscending)) << "Lower bounds missmatch in node with key " << std::to_string(expected->key);
			ASSERT_NO_FATAL_FAILURE(assertThatLiteralBoundsAndAssociatedClausesAreEqual(expected->upperBoundsSortedDescending, actual->upperBoundsSortedDescending)) << "Upper bounds missmatch in node with key " << std::to_string(expected->key);

			if (!expected->parent)
			{
				ASSERT_FALSE(actual->parent);
				return;
			}
			ASSERT_EQ(expected->parent->key, actual->parent->key) << "Parent key missmatch in node with key" << std::to_string(expected->key);
			ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expected->left, actual->left)) << "Left child missmatch in node with key " << std::to_string(expected->key);
			ASSERT_NO_FATAL_FAILURE(assertThatNodesAreEqual(expected->right, actual->right)) << "Right child missmatch in node with key " << std::to_string(expected->key);
		}

		static void assertThatLiteralBoundsAndAssociatedClausesAreEqual(const std::vector<avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair>& expected, const std::vector<avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair>& actual)
		{
			ASSERT_EQ(expected.size(), actual.size());
			for (std::size_t i = 0; i < expected.size(); ++i)
			{
				assertThatLiteralBoundAndAssociatedClauseIdxMatch(expected.at(i), actual.at(i));
			}
		}

		static void assertThatLiteralBoundAndAssociatedClauseIdxMatch(const avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair& expected, const avl::AvlIntervalTreeNode::LiteralBoundsAndClausePair& actual)
		{
			ASSERT_EQ(expected.literalBound, actual.literalBound);
			ASSERT_EQ(expected.idxOfReferencedClauseInFormula, actual.idxOfReferencedClauseInFormula);
		}

		static void assertThatClauseIndicesOfOverlappingIntervalsMatch(const std::vector<std::size_t>& expectedClauseIndices, const std::vector<std::size_t>& actualClauseIndices)
		{
			ASSERT_EQ(expectedClauseIndices.size(), actualClauseIndices.size());
			for (std::size_t i = 0; i < expectedClauseIndices.size(); ++i)
			{
				ASSERT_EQ(expectedClauseIndices.at(i), actualClauseIndices.at(i));
			}
		}
	};
}
#endif