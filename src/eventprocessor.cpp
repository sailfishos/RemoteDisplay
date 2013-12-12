#include "eventprocessor.h"
#include <freerdp/freerdp.h>
#include <QDebug>

EventProcessor::EventProcessor(freerdp *instance)
    : freeRdpInstance(instance) {
}

EventProcessor::~EventProcessor() {
}

void EventProcessor::requestStop() {
    stop = true;
}

void EventProcessor::run() {
    stop = false;
    int index;
    int rcount;
    int wcount;
    void* rfds[32];
    void* wfds[32];
    int fds_count;
    HANDLE fds[64];

    memset(rfds, 0, sizeof(rfds));
    memset(wfds, 0, sizeof(wfds));

    if (!freerdp_connect(freeRdpInstance)) {
        qDebug() << "Failed to connect";
        return;
    }

    while(!stop) {
        rcount = 0;
        wcount = 0;

        if (!freerdp_get_fds(freeRdpInstance, rfds, &rcount, wfds, &wcount)) {
            fprintf(stderr, "Failed to get FreeRDP file descriptor\n");
            break;
        }

        fds_count = 0;
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
            break;
        }

        // do the wait
        if (MsgWaitForMultipleObjects(fds_count, fds, FALSE, 1000, QS_ALLINPUT) == WAIT_FAILED) {
            fprintf(stderr, "wfreerdp_run: WaitForMultipleObjects failed: 0x%04X\n", GetLastError());
            break;
        }

        if (!freerdp_check_fds(freeRdpInstance)) {
            fprintf(stderr, "Failed to check FreeRDP file descriptor\n");
            break;
        }
        if (freerdp_shall_disconnect(freeRdpInstance)) {
            break;
        }
    }

    freerdp_disconnect(freeRdpInstance);
}
