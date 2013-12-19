#include "freerdphelpers.h"
#include <QDebug>

MyContext::MyContext() : self(nullptr), cursorChangeNotifier(nullptr) {
}

MyContext* getMyContext(rdpContext* context) {
    return reinterpret_cast<MyContext*>(context);
}

MyContext* getMyContext(freerdp* instance) {
    return getMyContext(instance->context);
}

QImage::Format bppToImageFormat(int bpp) {
    switch (bpp) {
    case 16:
        return QImage::Format_RGB16;
    case 24:
        return QImage::Format_RGB888;
    case 32:
        return QImage::Format_RGB32;
    }
    qWarning() << "Cannot handle" << bpp << "bits per pixel!";
    return QImage::Format_Invalid;
}
