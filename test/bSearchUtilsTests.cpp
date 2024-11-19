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


TEST_F(BSearchUtilsTests, SearchForFirstElementLargerThanXInAscendinglySortedContainer)
{
	const std::vector<long> containerToSearchThrough = { -3,-2,-2,-1,-1,1 };

	constexpr std::size_t expectedIndexOfElementLargerThanLargestElementInContainer = 6;
	constexpr std::size_t expectedIndexOfElementSmallerThanSmallestElementInContainer = 0;
	constexpr std::size_t expectedIndexOfIntermediateElementEqualToElementInContainer = 5;
	constexpr std::size_t expectedIndexOfIntermediateElementNotEqualToElementInContainer = 5;
	constexpr std::size_t expectedIndexOfElementEqualToLowestBound = 1;
	constexpr std::size_t expectedIndexOfElementEqualToLargestBound = 6;

	std::size_t actualIndexOfElementLargerThanLargestElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfElementSmallerThanSmallestElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfIntermediateElementEqualToElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfIntermediateElementNotEqualToElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfElementEqualToLowestBound = SIZE_MAX;
	std::size_t actualIndexOfElementEqualToLargestBound = SIZE_MAX;

	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementSmallerThanSmallestElementInContainer = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(-5)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementLargerThanLargestElementInContainer = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(3)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfIntermediateElementEqualToElementInContainer = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(-1)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfIntermediateElementNotEqualToElementInContainer = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(0)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementEqualToLowestBound = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(-3)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementEqualToLargestBound = bSearchUtils::bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(containerToSearchThrough, static_cast<long>(1)));

	ASSERT_EQ(expectedIndexOfElementLargerThanLargestElementInContainer, actualIndexOfElementLargerThanLargestElementInContainer);
	ASSERT_EQ(expectedIndexOfElementSmallerThanSmallestElementInContainer, actualIndexOfElementSmallerThanSmallestElementInContainer);
	ASSERT_EQ(expectedIndexOfIntermediateElementEqualToElementInContainer, actualIndexOfIntermediateElementEqualToElementInContainer);
	ASSERT_EQ(expectedIndexOfIntermediateElementNotEqualToElementInContainer, actualIndexOfIntermediateElementNotEqualToElementInContainer);
	ASSERT_EQ(expectedIndexOfElementEqualToLowestBound, actualIndexOfElementEqualToLowestBound);
	ASSERT_EQ(expectedIndexOfElementEqualToLargestBound, actualIndexOfElementEqualToLargestBound);
}

TEST_F(BSearchUtilsTests, SearchForFirstElementSmallerThanXInDescendinglySortedContainer)
{
	const std::vector<long> containerToSearchThrough = { 3,2,2,1,1,-1 };

	constexpr std::size_t expectedIndexOfElementLargerThanLargestElementInContainer = 0;
	constexpr std::size_t expectedIndexOfElementSmallerThanSmallestElementInContainer = 6;
	constexpr std::size_t expectedIndexOfIntermediateElementEqualToElementInContainer = 5;
	constexpr std::size_t expectedIndexOfIntermediateElementNotEqualToElementInContainer = 5;
	constexpr std::size_t expectedIndexOfElementEqualToLowestBound = 6;
	constexpr std::size_t expectedIndexOfElementEqualToLargestBound = 1;

	std::size_t actualIndexOfElementLargerThanLargestElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfElementSmallerThanSmallestElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfIntermediateElementEqualToElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfIntermediateElementNotEqualToElementInContainer = SIZE_MAX;
	std::size_t actualIndexOfElementEqualToLowestBound = SIZE_MAX;
	std::size_t actualIndexOfElementEqualToLargestBound = SIZE_MAX;

	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementLargerThanLargestElementInContainer = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(5)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementSmallerThanSmallestElementInContainer = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(-3)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfIntermediateElementEqualToElementInContainer = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(1)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfIntermediateElementNotEqualToElementInContainer = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(0)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementEqualToLowestBound = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(-1)));
	ASSERT_NO_FATAL_FAILURE(actualIndexOfElementEqualToLargestBound = bSearchUtils::bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(containerToSearchThrough, static_cast<long>(3)));

	ASSERT_EQ(expectedIndexOfElementLargerThanLargestElementInContainer, actualIndexOfElementLargerThanLargestElementInContainer);
	ASSERT_EQ(expectedIndexOfElementSmallerThanSmallestElementInContainer, actualIndexOfElementSmallerThanSmallestElementInContainer);
	ASSERT_EQ(expectedIndexOfIntermediateElementEqualToElementInContainer, actualIndexOfIntermediateElementEqualToElementInContainer);
	ASSERT_EQ(expectedIndexOfIntermediateElementNotEqualToElementInContainer, actualIndexOfIntermediateElementNotEqualToElementInContainer);
	ASSERT_EQ(expectedIndexOfElementEqualToLowestBound, actualIndexOfElementEqualToLowestBound);
	ASSERT_EQ(expectedIndexOfElementEqualToLargestBound, actualIndexOfElementEqualToLargestBound);
}