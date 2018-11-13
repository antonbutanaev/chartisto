#pragma once

#include <optional>
#include <functional>
#include <memory>
#include <type_traits>

namespace util {

template<class Item> using FuncItreator = std::function<Item*()>;

template <class Func, class OnItem> void iterateFunc(Func func, const OnItem &onItem) {
	for (;;) {
		auto item = func();
		if (!item)
			break;
		onItem(*item);
	}
}

template<class Value> auto funcRangeIterator(Value begin, Value end) {
	return [i = std::move(begin), end = std::move(end)] () mutable {
		std::optional<decltype(i)> result;
		if (i != end)
			result = i++;
		return result;
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
		if (it != end)
			result = itemTransform(*it++);
		return result;
	};
}

template <class Func1, class Func2> auto funcPairIterator(Func1 func1, Func2 func2) {
	auto func1Ptr = std::make_shared<Func1>(func1);
	return [
		value1 = (*func1Ptr)(),
		value2 = func2(),
		func1Ptr = std::move(func1Ptr),
		func1 = std::move(func1),
		func2 = std::move(func2),
		first = true
	] () mutable {
		std::optional<std::pair<
			std::remove_reference_t<decltype(*value1)>,
			std::remove_reference_t<decltype(*value2)>
		>> result;
		if (first) {
			first = false;
			if (value1 && value2)
				result = {*value1, *value2};
		} else {
			value1 = (*func1Ptr)();
			if (value1)
				result = {*value1, *value2};
			else {
				func1Ptr = std::make_shared<Func1>(func1);
				value1 = (*func1Ptr)();
				value2 = func2();
				if (value1 && value2)
					result = {*value1, *value2};
			}
		}
		return result;
	};
}

}
