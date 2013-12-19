#ifndef FREERDPCLIENT_H
#define FREERDPCLIENT_H

#include <QWidget>
#include <QMutex>
#include <QPointer>
#include <freerdp/freerdp.h>

class FreeRdpEventLoop;
class Cursor;

class FreeRdpClient : public QObject {
    Q_OBJECT
public:
    FreeRdpClient();
    ~FreeRdpClient();

    void paintDesktopTo(QPaintDevice *device, const QRect &rect);

    void sendMouseMoveEvent(int x, int y);
    void sendMousePressEvent(Qt::MouseButton button, int x, int y);
    void sendMouseReleaseEvent(Qt::MouseButton button, int x, int y);

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
    void sendMouseEvent(UINT16 flags, int x, int y);

    static void BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates);
    static BOOL PreConnectCallback(freerdp* instance);
    static BOOL PostConnectCallback(freerdp* instance);
    static void PostDisconnectCallback(freerdp* instance);

    freerdp* freeRdpInstance;
    QMutex offScreenBufferMutex;
    QImage offScreenBuffer;
    QPointer<FreeRdpEventLoop> loop;
};

#endif // FREERDPCLIENT_H
