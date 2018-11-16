#pragma once

#include <unordered_set>

namespace util {

class HashCombine {
public:
	template<class T> HashCombine &operator<<(const T &value) {
		h_ ^= std::hash<T>()(value) + 0x9e3779b9 + (h_ << 6) + (h_ >> 2);
		return *this;
	}
	unsigned value() const {return h_;}
private:
	unsigned h_ = 0;
};

}
