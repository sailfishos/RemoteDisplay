#include "freerdpclient.h"
#include "freerdpeventloop.h"
#include "cursorchangenotifier.h"
#include "freerdphelpers.h"

#include <freerdp/freerdp.h>
#include <freerdp/input.h>
#include <freerdp/utils/tcp.h>
#include <freerdp/codec/bitmap.h>
#include <freerdp/cache/pointer.h>

#include <QDebug>
#include <QPainter>

namespace {

UINT16 qtMouseButtonToRdpButton(Qt::MouseButton button) {
    if (button == Qt::LeftButton) {
        return PTR_FLAGS_BUTTON1;
    } else if (button == Qt::RightButton) {
        return PTR_FLAGS_BUTTON2;
    }
    return 0;
}

}

BOOL FreeRdpClient::PreConnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->aboutToConnect();
    return TRUE;
}

BOOL FreeRdpClient::PostConnectCallback(freerdp* instance) {
    auto context = getMyContext(instance);
    auto self = context->self;
    emit self->connected();
    pointer_cache_register_callbacks(instance->update);

    auto notifier = new CursorChangeNotifier(context, self);
    connect(notifier, SIGNAL(cursorChanged(Cursor)), self, SIGNAL(cursorChanged(Cursor)));

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

void FreeRdpClient::sendMouseMoveEvent(int x, int y) {
    sendMouseEvent(PTR_FLAGS_MOVE, x, y);
}

void FreeRdpClient::sendMousePressEvent(Qt::MouseButton button, int x, int y) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton | PTR_FLAGS_DOWN, x, y);
}

void FreeRdpClient::sendMouseReleaseEvent(Qt::MouseButton button, int x, int y) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton, x, y);
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

    auto context = freeRdpInstance->context;
    context->cache = cache_new(freeRdpInstance->settings);

    if (!freerdp_connect(freeRdpInstance)) {
        qDebug() << "Failed to connect";
        return;
    }

    loop->exec(freeRdpInstance);

    freerdp_disconnect(freeRdpInstance);

    if (context->cache) {
        cache_free(context->cache);
    }
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

void FreeRdpClient::sendMouseEvent(UINT16 flags, int x, int y) {
    // note that this method is called from another thread, so lots of checking
    // is needed, perhaps we would need a mutex as well?
    if (freeRdpInstance) {
        auto input = freeRdpInstance->input;
        if (input && input->MouseEvent) {
            input->MouseEvent(input, flags, x, y);
        }
    }
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
