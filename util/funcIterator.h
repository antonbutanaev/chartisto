#pragma once

#include <optional>

namespace util {

template <class Func, class OnItem> void iterateFunc(Func func, const OnItem &onItem) {
	for (;;) {
		auto item = func();
		if (!item)
			break;
		onItem(*item);
	}
}

template<class Value> auto funcRangeIterator(Value begin, Value end) {
	return [i = begin, end] () mutable {
		std::optional<decltype(i)> result;
		return i == end? result : result = i++;
	};
}

template<class Container> auto funcIterator(Container &container) {
	return [it = container.begin(), end = container.end()] () mutable {
		return it == end? nullptr : &*it++;
	};
}

template<class Container, class ItemPartAddr> auto funcIterator(Container &container, ItemPartAddr itemPartAddr) {
	return [it = container.begin(), end = container.end(), itemPartAddr = std::move(itemPartAddr)] () mutable {
		return it == end? nullptr : itemPartAddr(*it++);
	};
}

template<class Container, class ItemTransform> auto funcIteratorTransform(Container &container, ItemTransform itemTransform) {
	return [it = container.begin(), end = container.end(), itemTransform = std::move(itemTransform)] () mutable {
		std::optional<decltype(itemTransform(*it))> result;
		return it == end? result : result = itemTransform(*it++);
	};
}

}
