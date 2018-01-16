#include <iostream>
#include "painter.h"

Painter::Painter() {
}

void Painter::paint(const Painter::DrawLine &) {
    // a1
}

void Painter::setCanvasSize(const Size &) {
    std::cout << "e2" << std::endl;
}
