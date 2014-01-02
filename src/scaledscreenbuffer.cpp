#include "scaledscreenbuffer.h"

#include <QImage>
#include <QSize>
#include <QTransform>

class ScaledScreenBufferPrivate {
public:
    ScreenBuffer *sourceBuffer;
    QSize scaledSize;
    QTransform coordinateTransform;
};

ScaledScreenBuffer::ScaledScreenBuffer(ScreenBuffer *source, QObject *parent)
    : QObject(parent), d_ptr(new ScaledScreenBufferPrivate) {
    Q_D(ScaledScreenBuffer);
    d->sourceBuffer = source;
    Q_ASSERT(d->sourceBuffer);

    d->scaledSize = d->sourceBuffer->createImage().size();
}

ScaledScreenBuffer::~ScaledScreenBuffer() {
    delete d_ptr;
}

QImage ScaledScreenBuffer::createImage() const {
    Q_D(const ScaledScreenBuffer);
    auto sourceImage = d->sourceBuffer->createImage();
    if (!sourceImage.isNull()) {
        return sourceImage.scaled(d->scaledSize, Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
    }
    return QImage();
}

void ScaledScreenBuffer::scaleToFit(const QSize &size) {
    Q_D(ScaledScreenBuffer);
    auto sourceImage = d->sourceBuffer->createImage();
    if (!sourceImage.isNull()) {
        QSize sourceSize = sourceImage.size();
        d->scaledSize = sourceSize;
        d->scaledSize.scale(size, Qt::KeepAspectRatio);

        qreal scaleX = (qreal)sourceSize.width() / (qreal)d->scaledSize.width();
        qreal scaleY = (qreal)sourceSize.height() / (qreal)d->scaledSize.height();
        d->coordinateTransform.reset();
        d->coordinateTransform.scale(scaleX, scaleY);
    }
}

QPoint ScaledScreenBuffer::mapToSource(const QPoint &point) const {
    Q_D(const ScaledScreenBuffer);
    return d->coordinateTransform.map(point);
}
