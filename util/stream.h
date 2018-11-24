#pragma once

#include <utility>
#include <stdexcept>

namespace util {

template<class S> class Stream {
public:
	template<class ...Args>
	Stream(const std::string &fileName, Args &&...args) : s_(fileName.c_str(), std::forward<Args>(args)...) {
		if (!s_)
			throw std::runtime_error("could not open: " + fileName);
	}

	operator S&() {return s_;}
	template<class U> Stream &operator<<(const U &u) {s_ << u; return *this;}
	template<class U> Stream &operator>>(U &u) {s_ >> u; return *this;}
private:
	S s_;
};

}
