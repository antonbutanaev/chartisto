#pragma once

#include <unordered_set>
#include <unordered_map>

namespace util {

class HashCombine {
public:
	template<class T> HashCombine &operator<<(const T &value) {
		const auto hash = std::hash<T>{}(value);
		if (first_) {
			first_ = false;
			hash_ = hash;
		} else
			hash_ ^= hash + 0x9e3779b9 + (hash_ << 6) + (hash_ >> 2);
		return *this;
	}
	unsigned hash() const {return hash_;}
private:
	unsigned hash_{0};
	bool first_{true};
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
