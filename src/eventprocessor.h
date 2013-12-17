#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include <QWidget>
#include <QMutex>
#include <freerdp/freerdp.h>

class EventProcessor : public QObject {
    Q_OBJECT
public:
    EventProcessor();
    ~EventProcessor();

    void requestStop();

    void paintDesktopTo(QPaintDevice *device, const QRect &rect);
    void setSettingServerHostName(const QString &host);
    void setSettingServerPort(quint16 port);
    void setSettingDesktopSize(quint16 width, quint16 height);

public slots:
    void run();

signals:
    void aboutToConnect();
    void connected();
    void disconnected();
    void desktopUpdated();

private:
    void initFreeRDP();
    bool handleFds();

    static void BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates);
    static BOOL PreConnectCallback(freerdp* instance);
    static BOOL PostConnectCallback(freerdp* instance);
    static void PostDisconnectCallback(freerdp* instance);

    freerdp* freeRdpInstance;
    QMutex offScreenBufferMutex;
    QImage offScreenBuffer;
    bool stop;
};

#endif // EVENTPROCESSOR_H
