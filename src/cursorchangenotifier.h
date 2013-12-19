#ifndef CURSORCHANGENOTIFIER_H
#define CURSORCHANGENOTIFIER_H

#include <QObject>
#include <QImage>
#include <QMetaType>
#include <freerdp/graphics.h>

struct MyContext;
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

class CursorChangeNotifier : public QObject
{
    Q_OBJECT
public:
    CursorChangeNotifier(MyContext *context, QObject *parent = 0);

signals:
    void cursorChanged(const Cursor &cursor);

private:
    static void myPointerCreate(rdpContext* context, rdpPointer* pointer);
    static void myPointerFree(rdpContext* context, rdpPointer* pointer);
    static void myPointerSet(rdpContext* context, rdpPointer* pointer);
    static void myPointerSetNull(rdpContext* context);
    static void myPointerSetDefault(rdpContext* context);
};

#endif // CURSORCHANGENOTIFIER_H
