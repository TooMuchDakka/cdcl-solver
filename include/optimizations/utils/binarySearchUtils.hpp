#ifndef BINARY_SEARCH_UTILS_HPP
#define BINARY_SEARCH_UTILS_HPP

namespace bSearchUtils {
	enum SortOrder {
		Descending,
		Ascending
	};

	template<typename ElemType>
	bool isElementOutsideOfRange(const std::vector<ElemType>& container, const std::optional<std::vector<std::size_t>>& optionalContainerIndirection, ElemType element, SortOrder sortOrderOfContainer)
	{
		if (container.empty())
			return true;

		const std::size_t firstSortedContainerIndirectionIndex = optionalContainerIndirection.has_value() ? optionalContainerIndirection->front() : 0;
		const std::size_t lastSortedContainerIndirectionIndex = optionalContainerIndirection.has_value() ? optionalContainerIndirection->back() : container.size() - 1;
		return (sortOrderOfContainer == SortOrder::Descending && (container[firstSortedContainerIndirectionIndex] < element || container[lastSortedContainerIndirectionIndex] > element))
			|| (sortOrderOfContainer == SortOrder::Ascending && (container[firstSortedContainerIndirectionIndex] > element || container[lastSortedContainerIndirectionIndex] < element));
	}

	template<typename ElemType>
	std::optional<std::size_t> bSearchInSortedContainer(const std::vector<ElemType>& container, const std::optional<std::vector<std::size_t>>& optionalContainerIndirection, ElemType element, SortOrder sortOrderOfContainer) {
		if (isElementOutsideOfRange(container, optionalContainerIndirection, element, sortOrderOfContainer))
			return std::nullopt;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = optionalContainerIndirection.has_value() ? container.at(optionalContainerIndirection->at(mid)) : container.at(mid);
			if (elementAtMidPosition == element)
				return mid;
			if (sortOrderOfContainer == SortOrder::Descending ? elementAtMidPosition > element : elementAtMidPosition < element)
				low = mid + 1;
			else
				high = mid - 1;
		}
		return std::nullopt;
	}

	template<typename ElemType>
	std::size_t bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(const std::vector<ElemType>& container, const std::vector<std::size_t>& containerIndexIndirections, ElemType element)
	{
		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(containerIndexIndirections.at(mid));
			if (elementAtMidPosition >= element)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return low;
	}

	template<typename ElemType>
	std::size_t bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(const std::vector<ElemType>& container, const std::vector<std::size_t>& containerIndexIndirections, ElemType element)
	{
		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(containerIndexIndirections.at(mid));
			if (elementAtMidPosition <= element)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return low;
	}
}
#endif