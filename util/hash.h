#pragma once

#include <string>
#include <cstdint>
#include <type_traits>

namespace util {
namespace hash {

class FNV1a {
public:
	FNV1a &operator<<(const std::string &v) {
		if (!v.empty())
			update(toPtr(&v.front()), toPtr(&v.back()) + 1);
		return *this;
	}

	template<class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
	FNV1a &operator<<(const T &v) {
		update(toPtr(v), toPtr(v) + sizeof(T));
		return *this;
	}

	uint32_t value() const  {return h_;}
	operator uint32_t() const {return value();}

private:
	template<class T> const uint8_t *toPtr(const T &v) {
		return static_cast<const uint8_t*>(static_cast<const void*>(&v));
	}

	void update(const uint8_t *begin, const uint8_t *end) {
		for (auto it = begin; it != end; ++it) {
			h_ ^= *it;
			h_ *= Prime;
		}
	}

	constexpr static uint32_t Offset = 2166136261;
	constexpr static uint32_t Prime = 16777619;
	uint32_t h_ = Offset;
};

}}
