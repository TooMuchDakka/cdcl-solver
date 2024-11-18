#include "gtest/gtest.h"
#include "optimizations/utils/binarySearchUtils.hpp"

class BSearchUtilsTests : public testing::Test {
public:
	static void assertBinarySearchResultMatchesExpectedOne(const std::vector<long>& container, long element, const std::optional<std::size_t>& optionalExpectedIndexOfElement, bSearchUtils::SortOrder sortOrderOfElementsInContainer)
	{
		std::optional<std::size_t> actualIndexOfElement;
		ASSERT_NO_FATAL_FAILURE(actualIndexOfElement = bSearchUtils::bSearchInSortedContainer(container, element, sortOrderOfElementsInContainer));
		ASSERT_EQ(optionalExpectedIndexOfElement.has_value(), actualIndexOfElement.has_value());

		if (optionalExpectedIndexOfElement.has_value())
			ASSERT_EQ(optionalExpectedIndexOfElement.value(), actualIndexOfElement.value());
	}
};

TEST_F(BSearchUtilsTests, SearchForSmallestElementInAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1 }, -3, 0, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForLargestElementInAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1,1,2 }, 2, 4, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForIntermediateElementInAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1,1,2 }, -1, 2, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForNotContainedElementInAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1,1,2 }, -5, std::nullopt, bSearchUtils::SortOrder::Ascending));
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1,1,2 }, 3, std::nullopt, bSearchUtils::SortOrder::Ascending));
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3,-2,-1,1,2 }, 0, std::nullopt, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForElementInContainerContainingOneElementAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3 }, -3, 0, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForElementInEmptyContainerInAscendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ }, -3, std::nullopt, bSearchUtils::SortOrder::Ascending));
}

TEST_F(BSearchUtilsTests, SearchForSmallestElementInDescendiglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, 3, 0, bSearchUtils::SortOrder::Descending));
}

TEST_F(BSearchUtilsTests, SearchForLargestElementInDescendiglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, -2, 4, bSearchUtils::SortOrder::Descending));
}

TEST_F(BSearchUtilsTests, SearchForIntermediateElementInDescendiglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, -1, 3, bSearchUtils::SortOrder::Descending));
}

TEST_F(BSearchUtilsTests, SearchForNotContainedElementInDescendiglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, -5, std::nullopt, bSearchUtils::SortOrder::Descending));
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, -3, std::nullopt, bSearchUtils::SortOrder::Descending));
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ 3,2,1,-1,-2 }, 0, std::nullopt, bSearchUtils::SortOrder::Descending));
}

TEST_F(BSearchUtilsTests, SearchForElementInContainerContainingOneElementDescendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ -3 }, -3, 0, bSearchUtils::SortOrder::Descending));
}

TEST_F(BSearchUtilsTests, SearchForElementInEmptyContainerInDescendinglySortedContainer) {
	ASSERT_NO_FATAL_FAILURE(assertBinarySearchResultMatchesExpectedOne({ }, -3, std::nullopt, bSearchUtils::SortOrder::Descending));
}
