#include "freerdpeventloop.h"
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

void FreeRdpFdsListenerThread::run() {
    while(!shouldQuit && handleFds());
}

bool FreeRdpFdsListenerThread::handleFds() {
    int rcount = 0;
    int wcount = 0;
    void* rfds[32];
    void* wfds[32];

    memset(rfds, 0, sizeof(rfds));
    memset(wfds, 0, sizeof(wfds));

    if (!freerdp_get_fds(freeRdpInstance, rfds, &rcount, wfds, &wcount)) {
        qWarning() << "Failed to get FreeRDP file descriptor";
        return false;
    }

    if (!waitFds(rfds, rcount, wfds, wcount)) {
        return false;
    }

    return true;
}

#if defined(Q_OS_WIN)

bool FreeRdpFdsListenerThread::waitFds(void** rfds, int rcount, void** wfds, int wcount) {
    int index;
    int fds_count = 0;
    HANDLE fds[64];

    // setup read fds
    for (index = 0; index < rcount; index++) {
        fds[fds_count++] = rfds[index];
    }

    // setup write fds
    for (index = 0; index < wcount; index++) {
        fds[fds_count++] = wfds[index];
    }

    // exit if nothing to do
    if (fds_count == 0) {
        qWarning() << "fds_count is zero";
        return false;
    }

    // do the wait
    auto status = MsgWaitForMultipleObjects(fds_count, fds, FALSE, 1000, QS_ALLINPUT);
    if (status == WAIT_FAILED) {
        qWarning() << "WaitForMultipleObjects failed:" << GetLastError();
        return false;
    }
    if (status >= WAIT_OBJECT_0 && status <= (WAIT_OBJECT_0 + fds_count - 1)) {
        emit eventReceived();
    }

    return true;
}

#elif defined(Q_OS_UNIX)

bool FreeRdpFdsListenerThread::waitFds(void** rfds, int rcount, void** wfds, int wcount) {
    int max_fds = 0;
    timeval timeout;
    fd_set rfds_set;
    fd_set wfds_set;
    int i;
    int fds;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    max_fds = 0;
    FD_ZERO(&rfds_set);
    FD_ZERO(&wfds_set);

    for (i = 0; i < rcount; i++) {
        fds = (int)(long)(rfds[i]);

        if (fds > max_fds) {
            max_fds = fds;
        }

        FD_SET(fds, &rfds_set);
    }

    if (max_fds == 0) {
        return false;
    }

    int status = select(max_fds + 1, &rfds_set, NULL, NULL, &timeout);

    if (status > 0) {
        emit eventReceived();
    } else if (status == -1) {
        // these are not really errors
        if (!((errno == EAGAIN) || (errno == EWOULDBLOCK) ||
            (errno == EINPROGRESS) || (errno == EINTR)))
        {
            qWarning() << "select failed:" << errno;
            return false;
        }
    }

    return true;
}

#else
#error Implementation missing for current platform!
#endif

FreeRdpEventLoop::FreeRdpEventLoop(QObject *parent) :
    QObject(parent), thread(nullptr) {
}

FreeRdpEventLoop::~FreeRdpEventLoop() {
    if (thread) {
        thread->shouldQuit = false;
        thread->wait();
        delete thread;
    }
}

void FreeRdpEventLoop::listen(freerdp *instance) {
    if (!thread) {
        thread = new FreeRdpFdsListenerThread;
        thread->shouldQuit = false;
        thread->freeRdpInstance = instance;
        connect(thread, SIGNAL(eventReceived()), this, SIGNAL(eventReceived()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}

void FreeRdpEventLoop::stopListen() {
    if (thread) {
        thread->shouldQuit = false;
    }
}

