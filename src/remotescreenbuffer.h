#ifndef REMOTESCREENBUFFER_H
#define REMOTESCREENBUFFER_H

#include <QObject>

class QImage;
class QRect;
class QByteArray;
class RemoteScreenBufferPrivate;

class RemoteScreenBuffer : public QObject {
    Q_OBJECT
public:
    RemoteScreenBuffer(quint16 width, quint16 height, quint8 bpp, QObject *parent = 0);
    ~RemoteScreenBuffer();

    QImage createImage() const;
    void addRectangle(const QRect &rect, const QByteArray &data);

private:
    Q_DECLARE_PRIVATE(RemoteScreenBuffer)
    RemoteScreenBufferPrivate* const d_ptr;
};

#endif // REMOTESCREENBUFFER_H
