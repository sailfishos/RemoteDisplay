#ifndef CURSORCHANGENOTIFIER_H
#define CURSORCHANGENOTIFIER_H

#include <QObject>
#include <QImage>
#include <QMetaType>

typedef struct rdp_pointer rdpPointer;
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
    CursorChangeNotifier(QObject *parent = 0);

    void addPointer(rdpPointer* pointer);
    void removePointer(rdpPointer* pointer);
    void changePointer(rdpPointer* pointer);

    int getPointerStructSize() const;

signals:
    void cursorChanged(const Cursor &cursor);
};

#endif // CURSORCHANGENOTIFIER_H
