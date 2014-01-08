#include "remotescreenbuffer.h"
#include "freerdphelpers.h"

#include <QImage>
#include <QDebug>
#include <QPainter>

#include <freerdp/codec/bitmap.h>

class RemoteScreenBufferPrivate {
public:
    RemoteScreenBufferPrivate(RemoteScreenBuffer *q) : q_ptr(q) {
    }

    void initBuffer() {
        Q_Q(RemoteScreenBuffer);
        Q_ASSERT(isSizeAndFormatValid());
        if (isSizeAndFormatValid()) {
            bufferData.resize(width * height * bpp);
            targetImage = q->createImage();
            targetImage.fill(0);
        }
    }

    bool isSizeAndFormatValid() const {
        return width > 0 && height > 0 && bppToImageFormat(bpp) != QImage::Format_Invalid;
    }

    QByteArray bufferData;
    quint16 width;
    quint16 height;
    quint8 bpp;
    QImage targetImage;

private:
    Q_DECLARE_PUBLIC(RemoteScreenBuffer)
    RemoteScreenBuffer* const q_ptr;
};

RemoteScreenBuffer::RemoteScreenBuffer(quint16 width, quint16 height, quint8 bpp, QObject *parent)
    : QObject(parent), d_ptr(new RemoteScreenBufferPrivate(this)) {
    Q_D(RemoteScreenBuffer);
    d->width = width;
    d->height = height;
    d->bpp = bpp;
    d->initBuffer();
}

RemoteScreenBuffer::~RemoteScreenBuffer() {
    delete d_ptr;
}

QImage RemoteScreenBuffer::createImage() const {
    Q_D(const RemoteScreenBuffer);
    return QImage((uchar*)d->bufferData.data(), d->width, d->height, bppToImageFormat(d->bpp));
}

void RemoteScreenBuffer::addRectangle(const QRect &rect, const QByteArray &data) {
    Q_D(RemoteScreenBuffer);
    // hack: ignore 1 pixel height rectangles as decompressing of those
    // sometimes cause a heap corruption, reason unknown
    if (rect.height() == 1) {
        return;
    }

    // decompress update's image data to 'imgData'
    QByteArray imgData;
    imgData.resize(rect.width() * rect.height() * (d->bpp / 8));
    if (!bitmap_decompress((BYTE*)data.data(), (BYTE*)imgData.data(), rect.width(), rect.height(), data.size(), d->bpp, d->bpp)) {
        qWarning() << "Bitmap update decompression failed";
    }

    QImage rectImg((uchar*)imgData.data(), rect.width(), rect.height(), bppToImageFormat(d->bpp));

    QPainter painter(&d->targetImage);
    painter.drawImage(rect, rectImg);
}

