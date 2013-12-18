#include "freerdpclient.h"
#include "freerdpeventloop.h"

#include <freerdp/freerdp.h>
#include <freerdp/utils/tcp.h>
#include <freerdp/codec/bitmap.h>

#include <QDebug>
#include <QPainter>

namespace {

struct MyContext
{
    rdpContext freeRdpContext;
    FreeRdpClient *self;
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

BOOL FreeRdpClient::PreConnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->aboutToConnect();
    return TRUE;
}

BOOL FreeRdpClient::PostConnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->connected();
    return TRUE;
}

void FreeRdpClient::PostDisconnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->disconnected();
}

void FreeRdpClient::BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates) {
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

FreeRdpClient::FreeRdpClient()
    : freeRdpInstance(nullptr) {
    freerdp_wsa_startup();
    loop = new FreeRdpEventLoop(this);
}

FreeRdpClient::~FreeRdpClient() {
    if (freeRdpInstance) {
        freerdp_context_free(freeRdpInstance);
        freerdp_free(freeRdpInstance);
        freeRdpInstance = nullptr;
    }
    freerdp_wsa_cleanup();
}

void FreeRdpClient::requestStop() {
    loop->quit();
}

void FreeRdpClient::paintDesktopTo(QPaintDevice *device, const QRect &rect) {
    auto self = getMyContext(freeRdpInstance)->self;
    if (self) {
        QMutexLocker locker(&self->offScreenBufferMutex);
        QPainter painter(device);
        painter.drawImage(rect, self->offScreenBuffer, rect);
    }
}

void FreeRdpClient::run() {

    initFreeRDP();

    if (!freerdp_connect(freeRdpInstance)) {
        qDebug() << "Failed to connect";
        return;
    }

    loop->exec(freeRdpInstance);

    freerdp_disconnect(freeRdpInstance);
}

void FreeRdpClient::initFreeRDP() {
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

void FreeRdpClient::setSettingServerHostName(const QString &host) {
    initFreeRDP();
    auto hostData = host.toLocal8Bit();
    auto settings = freeRdpInstance->context->settings;
    free(settings->ServerHostname);
    settings->ServerHostname = _strdup(hostData.data());
}

void FreeRdpClient::setSettingServerPort(quint16 port) {
    initFreeRDP();
    auto settings = freeRdpInstance->context->settings;
    settings->ServerPort = port;
}

void FreeRdpClient::setSettingDesktopSize(quint16 width, quint16 height) {
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
