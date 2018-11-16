#pragma once

#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

namespace util {

template<class T> class Hasher {
private:
	template<class U>
	unsigned hash(const U &value, decltype(std::hash<U>{}({}))) const {return std::hash<U>{}(value);}

	template<class U>
	unsigned hash(const U &value, decltype(U{}.hash())) const {return value.hash();}
public:
	unsigned operator()(const T &value) const {return hash(value, 0);}
};

class HashCombine {
public:
	template<class T> HashCombine &operator<<(const T &value) {
		const auto hash = Hasher<T>{}(value);
		if (!hash_)
			hash_ = hash;
		else
			*hash_ ^= hash + 0x9e3779b9 + (*hash_ << 6) + (*hash_ >> 2);
		return *this;
	}
	unsigned hash() const {
		if (!hash_)
			throw std::runtime_error("hash not combined");
		return *hash_;
	}
private:
	std::optional<unsigned> hash_;
};

template<class K, class V>
using umap = std::unordered_map<K, V, Hasher<K>>;

template<class K>
using uset = std::unordered_set<K, Hasher<K>>;

}
