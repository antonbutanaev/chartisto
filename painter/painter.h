#ifndef PAINTER_H
#define PAINTER_H

#include <functional>

class Painter {

public:
    Painter();

    struct Point {
        int x,y;
    };

    struct Size {
      int w,h;
    };

    struct Rect {
      Point topLeft;
      Size size;
    };

    void setCanvasSize(const Size &);

    using DrawLine = std::function<void(const Rect&)>;
    void paint(const DrawLine&);
};

#endif // PAINTER_H
