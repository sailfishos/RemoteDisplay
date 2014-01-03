#include "cursorchangenotifier.h"
#include "freerdphelpers.h"
#include <freerdp/freerdp.h>
#include <freerdp/codec/color.h>
#include <QCursor>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QMetaType>
#include <QMap>
#include <QMutex>

namespace {

struct CursorData {
    CursorData(const QImage &image, const QImage &mask, int hotX, int hotY)
        : image(image), mask(mask), hotX(hotX), hotY(hotY) {
    }

    QImage image;
    QImage mask;
    int hotX;
    int hotY;
};

struct MyPointer {
    rdpPointer pointer;
    int index;
};

MyPointer* getMyPointer(rdpPointer* pointer) {
    return reinterpret_cast<MyPointer*>(pointer);
}

}

class CursorChangeNotifierPrivate {
public:
    CursorChangeNotifierPrivate() : cursorDataIndex(0) {
    }

    QMap<int,CursorData*> cursorDataMap;
    int cursorDataIndex;
    QMutex mutex;
};

CursorChangeNotifier::CursorChangeNotifier(QObject *parent)
    : QObject(parent), d_ptr(new CursorChangeNotifierPrivate) {
}

CursorChangeNotifier::~CursorChangeNotifier() {
    delete d_ptr;
}

void CursorChangeNotifier::addPointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);
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

    QMutexLocker(&d->mutex);
    d->cursorDataMap[d->cursorDataIndex] = new CursorData(image, mask, pointer->xPos, pointer->yPos);
    getMyPointer(pointer)->index = d->cursorDataIndex;
    d->cursorDataIndex++;
}

void CursorChangeNotifier::removePointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);
    QMutexLocker(&d->mutex);
    delete d->cursorDataMap.take(getMyPointer(pointer)->index);
}

void CursorChangeNotifier::changePointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);
    // pass the changed pointer index from RDP thread to GUI thread because
    // instances of QCursor should not created outside of GUI thread
    int index = getMyPointer(pointer)->index;
    QMetaObject::invokeMethod(this, "onPointerChanged", Q_ARG(int, index));
}

void CursorChangeNotifier::onPointerChanged(int index) {
    Q_D(CursorChangeNotifier);
    QMutexLocker(&d->mutex);
    if (d->cursorDataMap.contains(index)) {
        auto data = d->cursorDataMap[index];
        auto imgPixmap = QPixmap::fromImage(data->image);
        auto maskBitmap = QBitmap::fromImage(data->mask);
        imgPixmap.setMask(maskBitmap);

        emit cursorChanged(QCursor(imgPixmap, data->hotX, data->hotY));
    }
}

int CursorChangeNotifier::getPointerStructSize() const {
    return sizeof(MyPointer);
}
