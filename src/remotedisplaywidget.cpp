#include "remotedisplaywidget.h"

#include <freerdp/freerdp.h>
#include <freerdp/utils/tcp.h>

#include <QDebug>

namespace {

struct MyContext
{
    rdpContext freeRdpContext;
    RemoteDisplayWidgetPrivate* pimpl;
};

}

class RemoteDisplayWidgetPrivate {
public:
    RemoteDisplayWidgetPrivate() : freeRdpInstance(nullptr) {
    }

    void initFreeRDP() {
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
    }

    void setSettingServerHostName(const QString &host) {
        auto hostData = host.toLocal8Bit();
        auto settings = freeRdpInstance->context->settings;
        free(settings->ServerHostname);
        settings->ServerHostname = _strdup(hostData.data());
    }

    void setSettingServerPort(quint16 port) {
        auto settings = freeRdpInstance->context->settings;
        settings->ServerPort = port;
    }

    static MyContext* getMyContext(freerdp* instance) {
        return reinterpret_cast<MyContext*>(instance->context);
    }

    static BOOL PreConnectCallback(freerdp* instance) {
        return getMyContext(instance)->pimpl->onConnect();
    }

    static BOOL PostConnectCallback(freerdp* instance) {
        return getMyContext(instance)->pimpl->onConnected();
    }

    static void PostDisconnectCallback(freerdp* instance) {
        getMyContext(instance)->pimpl->onDisconnected();
    }

    bool onConnect() {
        qDebug() << "ON CONNECT";
        return true;
    }

    bool onConnected() {
        qDebug() << "ON CONNECTED";
        return true;
    }

    void onDisconnected() {
        qDebug() << "ON DISCONNECTED";
    }

    freerdp* freeRdpInstance;
};

typedef RemoteDisplayWidgetPrivate Pimpl;

RemoteDisplayWidget::RemoteDisplayWidget(QWidget *parent)
    : QWidget(parent), d_ptr(new RemoteDisplayWidgetPrivate) {
    freerdp_wsa_startup();
}

RemoteDisplayWidget::~RemoteDisplayWidget() {
    Q_D(RemoteDisplayWidget);
    if (d->freeRdpInstance) {
        freerdp_disconnect(d->freeRdpInstance);
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

    // TODO: find non-blocking alternative
    if (!freerdp_connect(d->freeRdpInstance)) {
        qDebug() << "Failed to connect";
        return;
    }
}
