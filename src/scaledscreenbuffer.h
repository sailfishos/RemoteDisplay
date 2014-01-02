#ifndef SCALEDSCREENBUFFER_H
#define SCALEDSCREENBUFFER_H

#include <QObject>
#include "screenbuffer.h"

class ScaledScreenBufferPrivate;
class QSize;
class QPoint;

/**
 * The ScaledScreenBuffer class is a wrapper which scales given source screen
 * buffer to fit to a given size while keeping its aspect ratio.
 *
 * The class also provides functionality to map coordinates in the scaled
 * buffer to coordinates in the source buffer.
 */
class ScaledScreenBuffer : public QObject, public ScreenBuffer {
    Q_OBJECT
public:
    ScaledScreenBuffer(ScreenBuffer *source, QObject *parent = 0);
    ~ScaledScreenBuffer();

    virtual QImage createImage() const;

    /**
     * Scales the screen buffer's dimensions to fit the given @a size.
     * When createImage() is called next time the returned image will be scaled
     * by keeping aspect ratio so that it fits within the @a size.
     */
    void scaleToFit(const QSize &size);

    /**
     * Maps given @a point in image returned by createImage() to a point in
     * the source screen buffer's image.
     */
    QPoint mapToSource(const QPoint &point) const;

private:
    Q_DECLARE_PRIVATE(ScaledScreenBuffer)
    ScaledScreenBufferPrivate* const d_ptr;
};

#endif // SCALEDSCREENBUFFER_H
