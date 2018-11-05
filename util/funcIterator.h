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
		std::optional<decltype(i)> ret;
		return i == end? ret : ret = i++;
	};
}

template<class Container> auto funcIterator(Container &container) {
	return [it = container.begin(), end = container.end()] () mutable {
		return it == end? nullptr : &*it++;
	};
}

}
