#include "cursorchangenotifier.h"
#include "freerdphelpers.h"
#include <freerdp/freerdp.h>
#include <freerdp/codec/color.h>
#include <QCursor>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QDebug>

namespace {

struct MyPointer {
    rdpPointer pointer;
    Cursor *myCursor;
};

MyPointer* getMyPointer(rdpPointer* pointer) {
    return reinterpret_cast<MyPointer*>(pointer);
}

}

Cursor::Cursor() : hotX(0), hotY(0){
}

Cursor::Cursor(const QImage &image, const QImage &mask, int hotX, int hotY)
    : image(image), mask(mask), hotX(hotX), hotY(hotY) {
}

Cursor::operator QCursor() const {
    auto imgPixmap = QPixmap::fromImage(image);
    auto maskBitmap = QBitmap::fromImage(mask);
    imgPixmap.setMask(maskBitmap);
    return QCursor(imgPixmap, hotX, hotY);
}


CursorChangeNotifier::CursorChangeNotifier(QObject *parent)
    : QObject(parent) {
    qRegisterMetaType<Cursor>();
}

void CursorChangeNotifier::addPointer(rdpPointer* pointer) {
    int w = pointer->width;
    int h = pointer->height;

    // build cursor image
    QImage image(w, h, bppToImageFormat(pointer->xorBpp));
    freerdp_image_flip(pointer->xorMaskData, image.bits(), w, h, image.depth());

    // build cursor's mask image
    auto data = new BYTE[pointer->lengthAndMask];
    freerdp_bitmap_flip(pointer->andMaskData, data, (w + 7) / 8, h);
    QImage mask(w, h, QImage::Format_Mono);
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            mask.setPixel(x, y, freerdp_get_pixel(data, x, y, w, h, 1));
        }
    }
    delete data;

    auto myPointer = getMyPointer(pointer);
    myPointer->myCursor = new Cursor(image, mask, pointer->xPos, pointer->yPos);
}

void CursorChangeNotifier::removePointer(rdpPointer* pointer) {
    Q_UNUSED(pointer);
    delete getMyPointer(pointer)->myCursor;
}

void CursorChangeNotifier::changePointer(rdpPointer* pointer) {
    auto myPointer = getMyPointer(pointer);
    emit cursorChanged(*myPointer->myCursor);
}

int CursorChangeNotifier::getPointerStructSize() const {
    return sizeof(MyPointer);
}
