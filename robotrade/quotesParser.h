#ifndef QUOTESPARSER_H
#define QUOTESPARSER_H

#include <iosfwd>
#include <chart/data.h>

namespace robotrade {

chart::data::PBars parse(std::istream&);

}

#endif
