#include "remotedisplaywidget.h"
#include "remotedisplaywidget_p.h"
#include "eventprocessor.h"

#include <freerdp/freerdp.h>
#include <freerdp/utils/tcp.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/codec/bitmap.h>

#include <QDebug>
#include <QThread>
#include <QPointer>
#include <QPainter>

namespace {

struct MyContext
{
    rdpContext freeRdpContext;
    RemoteDisplayWidgetPrivate* pimpl;
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

}

RemoteDisplayWidgetPrivate::RemoteDisplayWidgetPrivate(RemoteDisplayWidget *q)
    : q_ptr(q), freeRdpInstance(nullptr) {
    processorThread = new QThread(q);
    processorThread->start();
}

void RemoteDisplayWidgetPrivate::initFreeRDP() {
    Q_Q(RemoteDisplayWidget);

    if (freeRdpInstance) {
        return;
    }
    freeRdpInstance = freerdp_new();

    freeRdpInstance->ContextSize = sizeof(MyContext);
    freeRdpInstance->ContextNew = nullptr;
    freeRdpInstance->ContextFree = nullptr;
    freeRdpInstance->PreConnect = PreConnectCallback;
    freeRdpInstance->PostConnect = PostConnectCallback;
    freeRdpInstance->Authenticate = nullptr;
    freeRdpInstance->VerifyCertificate = nullptr;
    freeRdpInstance->VerifyChangedCertificate = nullptr;
    freeRdpInstance->LogonErrorInfo = nullptr;
    freeRdpInstance->PostDisconnect = PostDisconnectCallback;
    freeRdpInstance->SendChannelData = nullptr;
    freeRdpInstance->ReceiveChannelData = nullptr;

    freerdp_context_new(freeRdpInstance);
    getMyContext(freeRdpInstance)->pimpl = this;

    auto update = freeRdpInstance->update;
    update->BeginPaint = BeginPaintCallback;
    update->BitmapUpdate = BitmapUpdateCallback;

    auto settings = freeRdpInstance->context->settings;
    settings->EmbeddedWindow = TRUE;
    qDebug() << "Window ID:" << q->winId();
    settings->ParentWindowId = (UINT64)q->winId();
}

void RemoteDisplayWidgetPrivate::setSettingServerHostName(const QString &host) {
    auto hostData = host.toLocal8Bit();
    auto settings = freeRdpInstance->context->settings;
    free(settings->ServerHostname);
    settings->ServerHostname = _strdup(hostData.data());
}

void RemoteDisplayWidgetPrivate::setSettingServerPort(quint16 port) {
    auto settings = freeRdpInstance->context->settings;
    settings->ServerPort = port;
}

MyContext* RemoteDisplayWidgetPrivate::getMyContext(freerdp* instance) {
    return reinterpret_cast<MyContext*>(instance->context);
}

MyContext* RemoteDisplayWidgetPrivate::getMyContext(rdpContext* context) {
    return reinterpret_cast<MyContext*>(context);
}

BOOL RemoteDisplayWidgetPrivate::PreConnectCallback(freerdp* instance) {
    QMetaObject::invokeMethod(getMyContext(instance)->pimpl, "onAboutToConnect");
    return TRUE;
}

BOOL RemoteDisplayWidgetPrivate::PostConnectCallback(freerdp* instance) {
    QMetaObject::invokeMethod(getMyContext(instance)->pimpl, "onConnected");
    return TRUE;
}

void RemoteDisplayWidgetPrivate::PostDisconnectCallback(freerdp* instance) {
    QMetaObject::invokeMethod(getMyContext(instance)->pimpl, "onDisconnected");
}

void RemoteDisplayWidgetPrivate::BeginPaintCallback(rdpContext* context) {
    QMetaObject::invokeMethod(getMyContext(context)->pimpl, "onBeginPaint");
}

void RemoteDisplayWidgetPrivate::BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates) {
    auto self = getMyContext(context)->pimpl;

    // this method is called from another thread so we need to protect
    // access to the queue
    QMutexLocker locker(&self->imageUpdateQueueMutex);

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

        ImageUpdate iUp;
        iUp.rect = QRect(update->destLeft, update->destTop, w, h);
        iUp.data = imgData;
        iUp.image = QImage((uchar*)iUp.data.data(), w, h, bppToImageFormat(bpp));
        self->imageUpdates.enqueue(iUp);
    }
    QMetaObject::invokeMethod(self->q_func(), "update");
}

void RemoteDisplayWidgetPrivate::onAboutToConnect() {
    Q_ASSERT(thread() == QThread::currentThread());
    qDebug() << "ON CONNECT";
}

void RemoteDisplayWidgetPrivate::onConnected() {
    Q_ASSERT(thread() == QThread::currentThread());
    qDebug() << "ON CONNECTED";
}

void RemoteDisplayWidgetPrivate::onDisconnected() {
    Q_ASSERT(thread() == QThread::currentThread());
    qDebug() << "ON DISCONNECTED";
}

void RemoteDisplayWidgetPrivate::onBeginPaint() {
    Q_Q(RemoteDisplayWidget);
    Q_ASSERT(thread() == QThread::currentThread());
    q->update();
}

typedef RemoteDisplayWidgetPrivate Pimpl;

RemoteDisplayWidget::RemoteDisplayWidget(QWidget *parent)
    : QWidget(parent), d_ptr(new RemoteDisplayWidgetPrivate(this)) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    freerdp_wsa_startup();
}

RemoteDisplayWidget::~RemoteDisplayWidget() {
    Q_D(RemoteDisplayWidget);
    if (d->eventProcessor) {
        d->eventProcessor->requestStop();
    }
    d->processorThread->quit();
    d->processorThread->wait();

    if (d->freeRdpInstance) {
        freerdp_context_free(d->freeRdpInstance);
        freerdp_free(d->freeRdpInstance);
        d->freeRdpInstance = nullptr;
    }

    freerdp_wsa_cleanup();
    delete d_ptr;
}

void RemoteDisplayWidget::connectToHost(const QString &host, quint16 port) {
    Q_D(RemoteDisplayWidget);

    d->initFreeRDP();
    d->setSettingServerHostName(host);
    d->setSettingServerPort(port);

    qDebug() << "Connecting to" << host << ":" << port;

    d->eventProcessor = new EventProcessor(d->freeRdpInstance);
    d->eventProcessor->moveToThread(d->processorThread);
    QMetaObject::invokeMethod(d->eventProcessor, "run");
}

void RemoteDisplayWidget::paintEvent(QPaintEvent *event) {
    Q_D(RemoteDisplayWidget);
    QMutexLocker locker(&d->imageUpdateQueueMutex);

    QPainter painter(this);
    while (!d->imageUpdates.isEmpty()) {
        auto iUp = d->imageUpdates.dequeue();
        painter.drawImage(iUp.rect, iUp.image);
    }
}
