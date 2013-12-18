#ifndef FREERDPCLIENT_H
#define FREERDPCLIENT_H

#include <QWidget>
#include <QMutex>
#include <QPointer>
#include <freerdp/freerdp.h>

class FreeRdpEventLoop;

class FreeRdpClient : public QObject {
    Q_OBJECT
public:
    FreeRdpClient();
    ~FreeRdpClient();

    void paintDesktopTo(QPaintDevice *device, const QRect &rect);

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
