#ifndef FREERDPEVENTLOOP_H
#define FREERDPEVENTLOOP_H

#include <QObject>
#include <freerdp/freerdp.h>

class FreeRdpEventLoop : public QObject {
    Q_OBJECT
public:
    FreeRdpEventLoop(QObject *parent = 0);

    void exec(freerdp* instance);
    void quit();

private:
    bool handleFds();
    bool waitFds(void **rfds, int rcount, void **wfds, int wcount);

    freerdp* freeRdpInstance;
    bool shouldQuit;
};

#endif // FREERDPEVENTLOOP_H
