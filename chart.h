#ifndef CHART_H
#define CHART_H

#include <cstddef>

namespace chart {

using Volume = double;
using Price = double;
const Price NoPrice = 0;

struct Bar {
    virtual ~Bar() = default;

    virtual Price open() const = 0;
    virtual Price close() const = 0;
    virtual Price high() const = 0;
    virtual Price low() const = 0;

    virtual Volume volume() const = 0;
};

class Chart {
public:
    virtual ~Chart() = default;

    virtual size_t numBars() const = 0;
    virtual const Bar &bar(size_t i) const = 0;
};

class Canvas {
public:
    struct Size {
      int x,y;
    };
    Canvas();

    void setCanvasSize(const Size &);
};

}

#endif // PAINTER_H
