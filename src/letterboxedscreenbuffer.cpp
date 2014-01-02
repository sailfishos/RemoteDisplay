#include "letterboxedscreenbuffer.h"

#include <QImage>
#include <QPainter>

class LetterboxedScreenBufferPrivate {
public:
    ScreenBuffer *sourceBuffer;
    QSize size;
    QRect sourceRect;
    QTransform coordinateTransform;
};

LetterboxedScreenBuffer::LetterboxedScreenBuffer(ScreenBuffer *source, QObject *parent)
    : QObject(parent), d_ptr(new LetterboxedScreenBufferPrivate) {
    Q_D(LetterboxedScreenBuffer);
    d->sourceBuffer = source;
    Q_ASSERT(d->sourceBuffer);
}

LetterboxedScreenBuffer::~LetterboxedScreenBuffer() {
    delete d_ptr;
}

QImage LetterboxedScreenBuffer::createImage() const {
    Q_D(const LetterboxedScreenBuffer);
    auto sourceImage = d->sourceBuffer->createImage();
    if (!sourceImage.isNull()) {
        QImage image(d->size, sourceImage.format());
        QPainter painter(&image);
        painter.fillRect(image.rect(), Qt::black);
        painter.drawImage(d->sourceRect, sourceImage);
        return image;
    }
    return QImage();
}

QPoint LetterboxedScreenBuffer::mapToSource(const QPoint &point) const {
    Q_D(const LetterboxedScreenBuffer);
    QPoint p = d->coordinateTransform.map(point);
    p.setX(qMin(qMax(p.x(), 0), d->sourceRect.width() - 1));
    p.setY(qMin(qMax(p.y(), 0), d->sourceRect.height() - 1));
    return p;
}

void LetterboxedScreenBuffer::resize(const QSize &size) {
    Q_D(LetterboxedScreenBuffer);
    d->size = size;
    auto sourceImage = d->sourceBuffer->createImage();
    if (!sourceImage.isNull()) {
        d->sourceRect.setSize(sourceImage.size());
        d->sourceRect.moveCenter(QPoint(size.width() / 2, size.height() / 2));

        d->coordinateTransform.reset();
        d->coordinateTransform.translate(-d->sourceRect.left(), -d->sourceRect.top());
    }
}
