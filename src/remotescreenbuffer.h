#ifndef REMOTESCREENBUFFER_H
#define REMOTESCREENBUFFER_H

#include <QObject>
#include "screenbuffer.h"
#include "bitmaprectanglesink.h"

class QImage;
class QRect;
class QByteArray;
class RemoteScreenBufferPrivate;

/**
 * The RemoteScreenBuffer class is a screen buffer which contains the remote
 * host's whole display area.
 *
 * With addRectangle() the RDP handling thread updates the screen buffer.
 *
 * With createImage() the GUI thread can request for a QImage which provides
 * access to the buffer.
 */
class RemoteScreenBuffer : public QObject, public ScreenBuffer, public BitmapRectangleSink {
    Q_OBJECT
public:
    /**
     * Creates new remote screen buffer with dimensions @a width and @a height
     * and count of bits per pixel in @a bpp.
     */
    RemoteScreenBuffer(quint16 width, quint16 height, quint8 bpp, QObject *parent = 0);
    ~RemoteScreenBuffer();

    virtual QImage createImage() const;

    /**
     * Implemented from BitmapRectangleSink. Adds given bitmap rectangle to the
     * screen buffer. It is expected that the data is an encoded bitmap which
     * can be decompressed with FreeRDP's bitmap_decompress().
     *
     * Note that this method is thread-safe.
     */
    virtual void addRectangle(const QRect &rect, const QByteArray &data);

private:
    Q_DECLARE_PRIVATE(RemoteScreenBuffer)
    RemoteScreenBufferPrivate* const d_ptr;
};

#endif // REMOTESCREENBUFFER_H
