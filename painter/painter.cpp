#include "painter.h"

#include <QDebug>


Painter::Painter() {
}

void Painter::paint(const Painter::DrawLine &) {
    // a1
}

void Painter::setCanvasSize(const Size &) {
    qDebug() << "e";
}
