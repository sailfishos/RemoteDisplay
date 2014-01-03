#ifndef CURSORCHANGENOTIFIER_H
#define CURSORCHANGENOTIFIER_H

#include <QObject>
#include "pointerchangesink.h"

class QCursor;
class CursorChangeNotifierPrivate;

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
    ~CursorChangeNotifier();

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
    void cursorChanged(const QCursor &cursor);

private slots:
    void onPointerChanged(int index);

private:
    Q_DECLARE_PRIVATE(CursorChangeNotifier)
    CursorChangeNotifierPrivate* const d_ptr;
};

#endif // CURSORCHANGENOTIFIER_H
