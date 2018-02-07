#ifndef QUOTESPARSER_H
#define QUOTESPARSER_H

#include <iosfwd>
#include <functional>
#include <chart/chart.h>

namespace robotrade {

struct Bar {
	const std::string ticker;
	const chart::data::Bar &bar;
};

using ParseCallback = std::function<void(const Bar&)>;
void parse(std::istream&, const ParseCallback&);
}

#endif
