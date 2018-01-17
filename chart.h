#ifndef PAINTER_H
#define PAINTER_H

namespace chart {

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
