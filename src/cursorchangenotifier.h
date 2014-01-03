#ifndef CURSORCHANGENOTIFIER_H
#define CURSORCHANGENOTIFIER_H

#include <QObject>
#include <QImage>
#include <QMetaType>

#include "pointerchangesink.h"

class QCursor;

class Cursor {
public:
    Cursor();
    Cursor(const QImage &image, const QImage &mask, int hotX, int hotY);
    operator QCursor() const;

private:
    QImage image;
    QImage mask;
    int hotX;
    int hotY;
};
Q_DECLARE_METATYPE(Cursor)

/**
 * The CursorChangeNotifier class notifies when mouse cursor's style should
 * be changed.
 *
 * RDP server passes the currently shown mouse cursor's style over network when
 * ever the style changes. This class receives those updates and emits the
 * changed cursor style so that it can be applied in the widget.
 */
class CursorChangeNotifier : public QObject, public PointerChangeSink {
    Q_OBJECT
public:
    CursorChangeNotifier(QObject *parent = 0);

    /**
     * Implemented from PointerChangeSink.
     */
    virtual int getPointerStructSize() const;

    /**
     * Implemented from PointerChangeSink.
     */
    virtual void addPointer(rdpPointer* pointer);

    /**
     * Implemented from PointerChangeSink.
     */
    virtual void removePointer(rdpPointer* pointer);

    /**
     * Implemented from PointerChangeSink.
     */
    virtual void changePointer(rdpPointer* pointer);

signals:
    /**
     * This signal is emitted when current mouse cursor style changes.
     */
    void cursorChanged(const Cursor &cursor);
};

#endif // CURSORCHANGENOTIFIER_H
