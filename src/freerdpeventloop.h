#ifndef FREERDPEVENTLOOP_H
#define FREERDPEVENTLOOP_H

#include <QThread>
#include <QPointer>
#include <freerdp/freerdp.h>

class FreeRdpFdsListenerThread;

class FreeRdpEventLoop : public QObject {
    Q_OBJECT
public:
    FreeRdpEventLoop(QObject *parent = 0);
    ~FreeRdpEventLoop();

    void listen(freerdp* instance);
    void stopListen();

signals:
    void eventReceived();

private:
    QPointer<FreeRdpFdsListenerThread> thread;
};

// TODO: move to private header file
class FreeRdpFdsListenerThread : public QThread {
    Q_OBJECT
public:
    virtual void run();
    bool handleFds();
    bool waitFds(void** rfds, int rcount, void** wfds, int wcount);

signals:
    void eventReceived();

public:
    freerdp *freeRdpInstance;
    bool shouldQuit;
};

#endif // FREERDPEVENTLOOP_H
