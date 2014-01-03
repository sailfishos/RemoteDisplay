#ifndef FREERDPCLIENT_H
#define FREERDPCLIENT_H

#include <QWidget>
#include <QPointer>
#include <freerdp/freerdp.h>

class FreeRdpEventLoop;
class Cursor;
class BitmapRectangleSink;
class PointerChangeSink;
class ScreenBuffer;

class FreeRdpClient : public QObject {
    Q_OBJECT
public:
    FreeRdpClient(PointerChangeSink *pointerSink);
    ~FreeRdpClient();

    void setBitmapRectangleSink(BitmapRectangleSink *sink);

    quint8 getDesktopBpp() const;

    void sendMouseMoveEvent(const QPoint &pos);
    void sendMousePressEvent(Qt::MouseButton button, const QPoint &pos);
    void sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &pos);

public slots:
    void setSettingServerHostName(const QString &host);
    void setSettingServerPort(quint16 port);
    void setSettingDesktopSize(quint16 width, quint16 height);

    void run();
    void requestStop();

signals:
    void aboutToConnect();
    void connected();
    void disconnected();
    void desktopUpdated();

private:
    void initFreeRDP();
    void sendMouseEvent(UINT16 flags, const QPoint &pos);

    static void BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates);
    static BOOL PreConnectCallback(freerdp* instance);
    static BOOL PostConnectCallback(freerdp* instance);
    static void PostDisconnectCallback(freerdp* instance);

    static void PointerNewCallback(rdpContext* context, rdpPointer* pointer);
    static void PointerFreeCallback(rdpContext* context, rdpPointer* pointer);
    static void PointerSetCallback(rdpContext* context, rdpPointer* pointer);

    freerdp* freeRdpInstance;
    BitmapRectangleSink *bitmapRectangleSink;
    PointerChangeSink *pointerChangeSink;
    QPointer<FreeRdpEventLoop> loop;
};

#endif // FREERDPCLIENT_H
