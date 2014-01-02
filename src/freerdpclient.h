#ifndef FREERDPCLIENT_H
#define FREERDPCLIENT_H

#include <QWidget>
#include <QPointer>
#include <freerdp/freerdp.h>

class FreeRdpEventLoop;
class Cursor;
class BitmapRectangleSink;
class ScreenBuffer;

class FreeRdpClient : public QObject {
    Q_OBJECT
public:
    FreeRdpClient();
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
    void cursorChanged(const Cursor &cursor);

private:
    void initFreeRDP();
    void sendMouseEvent(UINT16 flags, const QPoint &pos);

    static void BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates);
    static BOOL PreConnectCallback(freerdp* instance);
    static BOOL PostConnectCallback(freerdp* instance);
    static void PostDisconnectCallback(freerdp* instance);

    freerdp* freeRdpInstance;
    BitmapRectangleSink *bitmapRectangleSink;
    QPointer<FreeRdpEventLoop> loop;
};

#endif // FREERDPCLIENT_H
