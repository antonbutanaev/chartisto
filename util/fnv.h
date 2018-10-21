#pragma once

#include <string>

namespace util {

class FNVHash {
public:
	template<class V> FNVHash &operator<<(V v) {
		h_ ^= static_cast<unsigned>(v);
		h_ *= 0x01000193;
		return *this;
	};

	FNVHash &operator<<(const std::string &v) {
		for (const auto &ch: v)
			*this << ch;
		return *this;
	}

	unsigned value() const  {return h_;}
	operator unsigned() const {return value();}

private:
	unsigned h_ = 0x811c9dc5;
};

}
