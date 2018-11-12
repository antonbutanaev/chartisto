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

template <class Func1, class Func2> auto funcPairIterator(Func1 func1, Func2 func2) {
	auto func1Ptr = std::make_shared<decltype(func1)>(func1);
	auto v1 = (*func1Ptr)();
	auto v2 = func2();
	return [
		func1Ptr = std::move(func1Ptr),
		func1Save = std::move(func1),
		func2 = std::move(func2),
		v1 = std::move(v1),
		v2 = std::move(v2),
		first = true
	] () mutable {
		std::optional<std::pair<decltype(*v1),decltype(*v2)>> ret;
		if (first) {
			first = false;
			if (v1 && v2)
				ret = {*v1, *v2};
		} else {
			v1 = (*func1Ptr)();
			if (v1)
				ret = {*v1, *v2};
			else {
				func1Ptr = std::make_shared<decltype(func1)>(func1Save);
				v1 = (*func1Ptr)();
				v2 = func2();
				if (v1 && v2)
					ret = {*v1, *v2};
			}
		}
		return ret;
	};
}

}
