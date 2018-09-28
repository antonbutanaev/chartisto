#pragma once

#include <cstddef>
#include <chart/data.h>

namespace robotrade {

class Levels {
};

void findLevels(chart::data::PBars bars, size_t from, size_t to, const std::string &config);

}
