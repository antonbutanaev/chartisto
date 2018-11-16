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

template<class HA, class H> auto combineTwoHashes(HA hashAdd, H hash) {
	return hash ^ (hashAdd + 0x9e3779b9 + (hash << 6) + (hash >> 2)); // boost hash_combine
}

template <class T> auto hashCombine(const T &value) {
	return Hasher<T>{}(value);
}

template<typename T, typename... Rest> auto hashCombine(const T &value, const Rest &...rest) {
	return combineTwoHashes(hashCombine(value), hashCombine(rest...));
}

class HashCombine {
public:
	template<class T> HashCombine &operator<<(const T &value) {
		hash_ = !hash_? hashCombine(value) : combineTwoHashes(hashCombine(value), *hash_);
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
