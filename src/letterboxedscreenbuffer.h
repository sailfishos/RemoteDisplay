#ifndef LETTERBOXEDSCREENBUFFER_H
#define LETTERBOXEDSCREENBUFFER_H

#include <QObject>
#include "screenbuffer.h"

class LetterboxedScreenBufferPrivate;
class QPoint;
class QSize;

/**
 * The LetterboxedScreenBuffer class is a wrapper which adds black borders
 * around source buffer if necessary.
 *
 * The class also provides functionality to map coordinates in this buffer
 * to coordinates in the source buffer.
 */
class LetterboxedScreenBuffer : public QObject, public ScreenBuffer {
    Q_OBJECT
public:
    LetterboxedScreenBuffer(ScreenBuffer *source, QObject *parent = 0);
    ~LetterboxedScreenBuffer();

    virtual QImage createImage() const;

    /**
     * Maps given @a point in image returned by createImage() to a point in
     * the source screen buffer's image.
     */
    QPoint mapToSource(const QPoint &point) const;

    /**
     * Resizes the screen buffer's dimensions to fit the given @a size.
     * Call createImage() to get the image with the new @a size.
     */
    void resize(const QSize &size);

private:
    Q_DECLARE_PRIVATE(LetterboxedScreenBuffer)
    LetterboxedScreenBufferPrivate* const d_ptr;
};

#endif // LETTERBOXEDSCREENBUFFER_H
