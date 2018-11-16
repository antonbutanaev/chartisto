#pragma once

#include <unordered_set>
#include <unordered_map>

namespace util {

class HashCombine {
public:
	template<class T> HashCombine &operator<<(const T &value) {
		h_ ^= std::hash<T>{}(value) + 0x9e3779b9 + (h_ << 6) + (h_ >> 2);
		return *this;
	}
	unsigned hash() const {return h_;}
private:
	unsigned h_ = 0;
};

template<class T> class Hasher {
private:
	template<class U>
	unsigned hash(const U &value, decltype(std::hash<U>{}({}))) const {return std::hash<U>{}(value);}

	template<class U>
	unsigned hash(const U &value, decltype(U{}.hash())) const {return value.hash();}
public:
	unsigned operator()(const T &value) const {return hash(value, 0);}
};

template<class K, class V>
using umap = std::unordered_map<K, V, Hasher<K>>;

template<class K>
using uset = std::unordered_set<K, Hasher<K>>;

}
