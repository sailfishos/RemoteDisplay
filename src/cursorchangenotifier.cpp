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


CursorChangeNotifier::CursorChangeNotifier(MyContext *context, QObject *parent)
    : QObject(parent) {
    qRegisterMetaType<Cursor>();

    Q_ASSERT(!context->cursorChangeNotifier);
    context->cursorChangeNotifier = this;

    rdpPointer pointer;
    memset(&pointer, 0, sizeof(rdpPointer));
    pointer.size = sizeof(MyPointer);
    pointer.New = myPointerCreate;
    pointer.Free = myPointerFree;
    pointer.Set = myPointerSet;
    pointer.SetNull = myPointerSetNull;
    pointer.SetDefault = myPointerSetDefault;
    graphics_register_pointer(context->freeRdpContext.graphics, &pointer);
}

void CursorChangeNotifier::myPointerCreate(rdpContext* context, rdpPointer* pointer) {
    Q_UNUSED(context);

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

void CursorChangeNotifier::myPointerFree(rdpContext* context, rdpPointer* pointer) {
    Q_UNUSED(context);
    Q_UNUSED(pointer);
    delete getMyPointer(pointer)->myCursor;
}

void CursorChangeNotifier::myPointerSet(rdpContext* context, rdpPointer* pointer) {
    auto self = getMyContext(context)->cursorChangeNotifier;
    auto myPointer = getMyPointer(pointer);
    emit self->cursorChanged(*myPointer->myCursor);
}

void CursorChangeNotifier::myPointerSetNull(rdpContext* context) {
    Q_UNUSED(context);
}

void CursorChangeNotifier::myPointerSetDefault(rdpContext* context) {
    Q_UNUSED(context);
}
