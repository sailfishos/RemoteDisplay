#include "freerdpeventloop.h"
#include <freerdp/channels/channels.h>
#include <QCoreApplication>

FreeRdpEventLoop::FreeRdpEventLoop(QObject *parent) :
    QObject(parent), freeRdpInstance(nullptr) {
}

void FreeRdpEventLoop::exec(freerdp *instance) {
    freeRdpInstance = instance;
    shouldQuit = false;

    while(!shouldQuit) {
        if (!handleFds()) {
            break;
        }
        QCoreApplication::processEvents();
    }
}

void FreeRdpEventLoop::quit() {
    shouldQuit = true;
}

bool FreeRdpEventLoop::handleFds() {
    int rcount = 0;
    int wcount = 0;
    void* rfds[32];
    void* wfds[32];

    memset(rfds, 0, sizeof(rfds));
    memset(wfds, 0, sizeof(wfds));

    auto channels = freeRdpInstance->context->channels;

    if (!freerdp_get_fds(freeRdpInstance, rfds, &rcount, wfds, &wcount)) {
        fprintf(stderr, "Failed to get FreeRDP file descriptor\n");
        return false;
    }

    if (!freerdp_channels_get_fds(channels, freeRdpInstance, rfds, &rcount, wfds, &wcount)) {
        fprintf(stderr, "Failed to get channel manager file descriptor\n");
        return false;
    }

    if (!waitFds(rfds, rcount, wfds, wcount)) {
        return false;
    }

    if (!freerdp_check_fds(freeRdpInstance)) {
        fprintf(stderr, "Failed to check FreeRDP file descriptor\n");
        return false;
    }

    if (!freerdp_channels_check_fds(channels, freeRdpInstance)) {
        fprintf(stderr, "Failed to check channel manager file descriptor\n");
        return false;
    }

    if (freerdp_shall_disconnect(freeRdpInstance)) {
        return false;
    }

    return true;
}

#if defined(Q_OS_WIN)

bool FreeRdpEventLoop::waitFds(void** rfds, int rcount, void** wfds, int wcount) {
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
        fprintf(stderr, "wfreerdp_run: fds_count is zero\n");
        return false;
    }

    // do the wait
    if (MsgWaitForMultipleObjects(fds_count, fds, FALSE, 1000, QS_ALLINPUT) == WAIT_FAILED) {
        fprintf(stderr, "wfreerdp_run: WaitForMultipleObjects failed: 0x%04X\n", GetLastError());
        return false;
    }

    return true;
}

#elif defined(Q_OS_UNIX)

bool FreeRdpEventLoop::waitFds(void** rfds, int rcount, void** wfds, int wcount) {
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

    int select_status = select(max_fds + 1, &rfds_set, NULL, NULL, &timeout);

    if (select_status == 0) {
        return true;
    } else if (select_status == -1) {
        /* these are not really errors */
        if (!((errno == EAGAIN) || (errno == EWOULDBLOCK) ||
            (errno == EINPROGRESS) || (errno == EINTR))) /* signal occurred */
        {
            fprintf(stderr, "xfreerdp_run: select failed\n");
            return false;
        }
    }

    return true;
}

#else
#error Implementation missing for current platform!
#endif
