#pragma once

#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <unordered_map>

namespace util {

template<class T> class Hasher {
private:
	template<class U>
	auto hash(const U &value) const -> decltype(std::hash<U>{}({})) {return std::hash<U>{}(value);}

	template<class U>
	auto hash(const U &value) const -> decltype(U{}.hash()) {return value.hash();}
public:
	auto operator()(const T &value) const {return hash(value);}
};

template<class S, class T> void hashCombine(S &seed, const T &value) {
	seed ^= Hasher<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2); // like boost hash_combine
}

template<class T> auto makeHash(const T &value) {
	return Hasher<T>{}(value);
}

template<class T, class ...Rest> auto makeHash(const T &value, const Rest &...rest) {
	auto seed = makeHash(rest...);
	hashCombine(seed, value);
	return seed;
}

template<class K, class V>
using umap = std::unordered_map<K, V, Hasher<K>>;

template<class K>
using uset = std::unordered_set<K, Hasher<K>>;

}
