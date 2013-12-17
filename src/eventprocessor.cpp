#include "eventprocessor.h"

#include <freerdp/freerdp.h>
#include <freerdp/utils/tcp.h>
#include <freerdp/codec/bitmap.h>

#include <QDebug>
#include <QPainter>

namespace {

struct MyContext
{
    rdpContext freeRdpContext;
    EventProcessor *self;
};

QImage::Format bppToImageFormat(int bpp) {
    switch (bpp) {
    case 16:
        return QImage::Format_RGB16;
    case 24:
        return QImage::Format_RGB888;
    case 32:
        return QImage::Format_RGB32;
    }
    qWarning() << "Cannot handle" << bpp << "bits per pixel!";
    return QImage::Format_Invalid;
}

MyContext* getMyContext(rdpContext* context) {
    return reinterpret_cast<MyContext*>(context);
}

MyContext* getMyContext(freerdp* instance) {
    return getMyContext(instance->context);
}

}

BOOL EventProcessor::PreConnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->aboutToConnect();
    return TRUE;
}

BOOL EventProcessor::PostConnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->connected();
    return TRUE;
}

void EventProcessor::PostDisconnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->disconnected();
}

void EventProcessor::BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates) {
    auto self = getMyContext(context)->self;

    QMutexLocker locker(&self->offScreenBufferMutex);
    QPainter painter(&self->offScreenBuffer);

    for (quint32 i = 0; i < updates->number; i++) {
        auto update = &updates->rectangles[i];

        if (!update->compressed) {
            qWarning() << "Handling uncompressed bitmap updates not implemented";
            continue;
        }

        quint32 w = update->width;
        quint32 h = update->height;
        quint32 bpp = update->bitsPerPixel;
        BYTE* srcData = update->bitmapDataStream;
        quint32 srcLength = update->bitmapLength;

        // decompress update's image data to 'imgData'
        QByteArray imgData;
        imgData.resize(w * h * (bpp / 8));
        if (!bitmap_decompress(srcData, (BYTE*)imgData.data(), w, h, srcLength, bpp, bpp)) {
            qWarning() << "Bitmap update decompression failed";
        }

        QRect rect(update->destLeft, update->destTop, w, h);
        QImage image((uchar*)imgData.data(), w, h, bppToImageFormat(bpp));
        painter.drawImage(rect, image);
    }
    emit self->desktopUpdated();
}

EventProcessor::EventProcessor()
    : freeRdpInstance(nullptr) {
    freerdp_wsa_startup();
}

EventProcessor::~EventProcessor() {
    if (freeRdpInstance) {
        freerdp_context_free(freeRdpInstance);
        freerdp_free(freeRdpInstance);
        freeRdpInstance = nullptr;
    }
    freerdp_wsa_cleanup();
}

void EventProcessor::requestStop() {
    stop = true;
}

void EventProcessor::paintDesktopTo(QPaintDevice *device, const QRect &rect) {
    auto self = getMyContext(freeRdpInstance)->self;
    if (self) {
        QMutexLocker locker(&self->offScreenBufferMutex);
        QPainter painter(device);
        painter.drawImage(rect, self->offScreenBuffer, rect);
    }
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

    initFreeRDP();

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

void EventProcessor::initFreeRDP() {
    if (freeRdpInstance) {
        return;
    }
    freeRdpInstance = freerdp_new();

    freeRdpInstance->ContextSize = sizeof(MyContext);
    freeRdpInstance->ContextNew = nullptr;
    freeRdpInstance->ContextFree = nullptr;
    freeRdpInstance->Authenticate = nullptr;
    freeRdpInstance->VerifyCertificate = nullptr;
    freeRdpInstance->VerifyChangedCertificate = nullptr;
    freeRdpInstance->LogonErrorInfo = nullptr;
    freeRdpInstance->SendChannelData = nullptr;
    freeRdpInstance->ReceiveChannelData = nullptr;
    freeRdpInstance->PreConnect = PreConnectCallback;
    freeRdpInstance->PostConnect = PostConnectCallback;
    freeRdpInstance->PostDisconnect = PostDisconnectCallback;

    freerdp_context_new(freeRdpInstance);
    getMyContext(freeRdpInstance)->self = this;

    auto update = freeRdpInstance->update;
    update->BitmapUpdate = BitmapUpdateCallback;

    auto settings = freeRdpInstance->context->settings;
    settings->EmbeddedWindow = TRUE;
}

void EventProcessor::setSettingServerHostName(const QString &host) {
    initFreeRDP();
    auto hostData = host.toLocal8Bit();
    auto settings = freeRdpInstance->context->settings;
    free(settings->ServerHostname);
    settings->ServerHostname = _strdup(hostData.data());
}

void EventProcessor::setSettingServerPort(quint16 port) {
    initFreeRDP();
    auto settings = freeRdpInstance->context->settings;
    settings->ServerPort = port;
}

void EventProcessor::setSettingDesktopSize(quint16 width, quint16 height) {
    initFreeRDP();
    auto settings = freeRdpInstance->settings;
    settings->DesktopWidth = width;
    settings->DesktopHeight = height;

    QMutexLocker locker(&offScreenBufferMutex);
    if (offScreenBuffer.isNull()) {
        offScreenBuffer = QImage(width, height, QImage::Format_RGB32);
        offScreenBuffer.fill(0);
    } else {
        offScreenBuffer = offScreenBuffer.copy(0, 0, width, height);
    }
}
