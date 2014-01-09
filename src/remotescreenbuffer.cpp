#include "remotescreenbuffer.h"
#include "freerdphelpers.h"

#include <QImage>
#include <QDebug>
#include <QPainter>
#include <QFile>

class RemoteScreenBufferPrivate {
public:
    RemoteScreenBufferPrivate(RemoteScreenBuffer *q) : q_ptr(q) {
    }

    void initBuffer(int bpp) {
        Q_Q(RemoteScreenBuffer);
        Q_ASSERT(isSizeAndFormatValid(bpp));
        if (isSizeAndFormatValid(bpp)) {
            bufferData.resize(width * height * bpp);
            targetImage = q->createImage();
            targetImage.fill(0);
        }
    }

    bool isSizeAndFormatValid(int bpp) const {
        return width > 0 && height > 0 && bppToImageFormat(bpp) != QImage::Format_Invalid;
    }

    QByteArray bufferData;
    quint16 width;
    quint16 height;
    QImage::Format format;
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
    d->format = bppToImageFormat(bpp);
    d->initBuffer(bpp);
}

RemoteScreenBuffer::~RemoteScreenBuffer() {
    delete d_ptr;
}

QImage RemoteScreenBuffer::createImage() const {
    Q_D(const RemoteScreenBuffer);
    return QImage((uchar*)d->bufferData.data(), d->width, d->height, d->format);
}

void RemoteScreenBuffer::addRectangle(const QRect &rect, const QByteArray &data) {
    Q_D(RemoteScreenBuffer);
    QImage rectImg((uchar*)data.data(), rect.width(), rect.height(), d->format);

    QPainter painter(&d->targetImage);
    painter.drawImage(rect, rectImg);
}
