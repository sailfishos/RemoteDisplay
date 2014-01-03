#include "freerdpclient.h"
#include "freerdpeventloop.h"
#include "freerdphelpers.h"
#include "bitmaprectanglesink.h"
#include "pointerchangesink.h"

#include <freerdp/freerdp.h>
#include <freerdp/input.h>
#include <freerdp/utils/tcp.h>
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
    auto settings = instance->context->settings;
    auto self = context->self;
    pointer_cache_register_callbacks(instance->update);

    rdpPointer pointer;
    memset(&pointer, 0, sizeof(rdpPointer));
    pointer.size = self->pointerChangeSink->getPointerStructSize();
    pointer.New = PointerNewCallback;
    pointer.Free = PointerFreeCallback;
    pointer.Set = PointerSetCallback;
    pointer.SetNull = NULL;
    pointer.SetDefault = NULL;
    graphics_register_pointer(context->freeRdpContext.graphics, &pointer);

    emit self->connected();

    return TRUE;
}

void FreeRdpClient::PostDisconnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->disconnected();
}

void FreeRdpClient::PointerNewCallback(rdpContext *context, rdpPointer *pointer) {
    getMyContext(context)->self->pointerChangeSink->addPointer(pointer);
}

void FreeRdpClient::PointerFreeCallback(rdpContext *context, rdpPointer *pointer) {
    getMyContext(context)->self->pointerChangeSink->removePointer(pointer);
}

void FreeRdpClient::PointerSetCallback(rdpContext *context, rdpPointer *pointer) {
    getMyContext(context)->self->pointerChangeSink->changePointer(pointer);
}

void FreeRdpClient::BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates) {
    auto self = getMyContext(context)->self;
    auto sink = self->bitmapRectangleSink;

    if (sink) {
        for (quint32 i = 0; i < updates->number; i++) {
            auto u = &updates->rectangles[i];
            QRect rect(u->destLeft, u->destTop, u->width, u->height);
            QByteArray data((char*)u->bitmapDataStream, u->bitmapLength);
            sink->addRectangle(rect, data);
        }
        emit self->desktopUpdated();
    }
}

FreeRdpClient::FreeRdpClient(PointerChangeSink *pointerSink)
    : freeRdpInstance(nullptr), bitmapRectangleSink(nullptr),
      pointerChangeSink(pointerSink) {
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

void FreeRdpClient::sendMouseMoveEvent(const QPoint &pos) {
    sendMouseEvent(PTR_FLAGS_MOVE, pos);
}

void FreeRdpClient::sendMousePressEvent(Qt::MouseButton button, const QPoint &pos) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton | PTR_FLAGS_DOWN, pos);
}

void FreeRdpClient::sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &pos) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton, pos);
}

void FreeRdpClient::setBitmapRectangleSink(BitmapRectangleSink *sink) {
    bitmapRectangleSink = sink;
}

quint8 FreeRdpClient::getDesktopBpp() const {
    if (freeRdpInstance && freeRdpInstance->settings) {
        return freeRdpInstance->settings->ColorDepth;
    }
    return 0;
}

void FreeRdpClient::run() {

    initFreeRDP();

    auto context = freeRdpInstance->context;
    context->cache = cache_new(freeRdpInstance->settings);

    if (!freerdp_connect(freeRdpInstance)) {
        qDebug() << "Failed to connect";
        emit disconnected();
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

void FreeRdpClient::sendMouseEvent(UINT16 flags, const QPoint &pos) {
    // note that this method is called from another thread, so lots of checking
    // is needed, perhaps we would need a mutex as well?
    if (freeRdpInstance) {
        auto input = freeRdpInstance->input;
        if (input && input->MouseEvent) {
            input->MouseEvent(input, flags, pos.x(), pos.y());
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
}
