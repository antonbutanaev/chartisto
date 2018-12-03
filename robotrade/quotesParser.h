#ifndef QUOTESPARSER_H
#define QUOTESPARSER_H

#include <functional>
#include <memory>
#include <chart/data.h>

namespace robotrade {

using TodayQuote = std::function<std::unique_ptr<std::istream>(const std::string &title)>;

chart::data::PBars parse(std::istream&, const TodayQuote& = {});

}

#endif
