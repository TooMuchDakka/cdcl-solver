#ifndef BINARY_SEARCH_UTILS_HPP
#define BINARY_SEARCH_UTILS_HPP

// https://stackoverflow.com/questions/6553970/find-the-first-element-in-a-sorted-array-that-is-greater-than-the-target?rq=3
namespace bSearchUtils {
	enum SortOrder {
		Descending,
		Ascending
	};

	template<typename ElemType>
	bool isElementOutsideOfRange(const std::vector<ElemType>& container, ElemType element, SortOrder sortOrderOfContainer)
	{
		return container.empty()
			|| (sortOrderOfContainer == SortOrder::Descending && (container.front() < element || container.back() > element))
				|| (sortOrderOfContainer == SortOrder::Ascending && (container.front() > element || container.back() < element));
	}

	template<typename ElemType>
	std::optional<std::size_t> bSearchInSortedContainer(const std::vector<ElemType>& container, ElemType element, SortOrder sortOrderOfContainer) {
		if (isElementOutsideOfRange(container, element, sortOrderOfContainer))
			return std::nullopt;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high && high != SIZE_MAX)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(mid);
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
	std::size_t bSearchForIndexOfLargestElementInSetOfElementsInLargerOrEqualThanXInDescendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	{
		if (container.empty())
			return 0;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high && high != SIZE_MAX)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(mid);
			if (element >= elementAtMidPosition)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return low;
	}

	template<typename ElemType>
	std::size_t bSearchForIndexOfSmallestElementInSetOfElementsSmallerOrEqualThanXInAscendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	{
		if (container.empty())
			return 0;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high && high != SIZE_MAX)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(mid);
			if (element <= elementAtMidPosition)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return low;
	}

	//template<typename ElemType>
	//std::size_t bSearchForIndexOfFirstElementSmallerThanXInAscendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	//{
	//	if (container.empty())
	//		return 0;

	//	std::size_t low = 0;
	//	std::size_t high = container.size() - 1;
	//	while (low <= high && high != SIZE_MAX)
	//	{
	//		const std::size_t mid = low + ((high - low) / 2);
	//		const ElemType elementAtMidPosition = container.at(mid);
	//		if (element < elementAtMidPosition)
	//			low = mid + 1;
	//		else
	//			high = mid - 1;
	//	}
	//	return low;
	//}

	//template<typename ElemType>
	//std::size_t bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	//{
	//	if (container.empty())
	//		return 0;

	//	std::size_t low = 0;
	//	std::size_t high = container.size() - 1;
	//	while (low <= high && high != SIZE_MAX)
	//	{
	//		const std::size_t mid = low + ((high - low) / 2);
	//		const ElemType elementAtMidPosition = container.at(mid);
	//		if (element > elementAtMidPosition)
	//			high = mid - 1;
	//		else
	//			low = mid + 1;
	//	}
	//	return low;
	//}


	template<typename ElemType>
	std::size_t bSearchForIndexOfLastElementLargerThanXInAscendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	{
		if (container.empty())
			return 0;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high && high != SIZE_MAX)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(mid);
			if (element >= elementAtMidPosition)
				low = mid + 1;
			else
				high = mid - 1;
		}
		return low;
	}

	template<typename ElemType>
	std::size_t bSearchForIndexOfFirstElementSmallerThanXInDescendinglySortedContainer(const std::vector<ElemType>& container, ElemType element)
	{
		if (container.empty())
			return 0;

		std::size_t low = 0;
		std::size_t high = container.size() - 1;
		while (low <= high && high != SIZE_MAX)
		{
			const std::size_t mid = low + ((high - low) / 2);
			const ElemType elementAtMidPosition = container.at(mid);
			if (element > elementAtMidPosition)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return low;
	}
}
#endif