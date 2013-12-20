#include "remotedisplaywidget.h"
#include "remotedisplaywidget_p.h"
#include "freerdpclient.h"
#include "cursorchangenotifier.h"

#include <QDebug>
#include <QThread>
#include <QPointer>
#include <QPaintEvent>

RemoteDisplayWidgetPrivate::RemoteDisplayWidgetPrivate(RemoteDisplayWidget *q)
    : q_ptr(q) {
}

void RemoteDisplayWidgetPrivate::onAboutToConnect() {
    qDebug() << "ON CONNECT";
}

void RemoteDisplayWidgetPrivate::onConnected() {
    qDebug() << "ON CONNECTED";
}

void RemoteDisplayWidgetPrivate::onDisconnected() {
    qDebug() << "ON DISCONNECTED";
}

void RemoteDisplayWidgetPrivate::onCursorChanged(const Cursor &cursor) {
    Q_Q(RemoteDisplayWidget);
    q->setCursor(cursor);
}

typedef RemoteDisplayWidgetPrivate Pimpl;

RemoteDisplayWidget::RemoteDisplayWidget(QWidget *parent)
    : QWidget(parent), d_ptr(new RemoteDisplayWidgetPrivate(this)) {
    Q_D(RemoteDisplayWidget);
    qRegisterMetaType<Qt::MouseButton>("Qt::MouseButton");

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);

    d->eventProcessor = new FreeRdpClient;

    connect(d->eventProcessor, SIGNAL(aboutToConnect()), d, SLOT(onAboutToConnect()));
    connect(d->eventProcessor, SIGNAL(connected()), d, SLOT(onConnected()));
    connect(d->eventProcessor, SIGNAL(disconnected()), d, SLOT(onDisconnected()));
    connect(d->eventProcessor, SIGNAL(desktopUpdated()), this, SLOT(update()));
    connect(d->eventProcessor, SIGNAL(cursorChanged(Cursor)), d, SLOT(onCursorChanged(Cursor)));
}

RemoteDisplayWidget::~RemoteDisplayWidget() {
    Q_D(RemoteDisplayWidget);
    if (d->eventProcessor) {
        d->eventProcessor->requestStop();
    }

    delete d_ptr;
}

void RemoteDisplayWidget::setDesktopSize(quint16 width, quint16 height) {
    Q_D(RemoteDisplayWidget);
    d->desktopSize = QSize(width, height);
    d->eventProcessor->setSettingDesktopSize(width, height);
}

void RemoteDisplayWidget::connectToHost(const QString &host, quint16 port) {
    Q_D(RemoteDisplayWidget);

    d->eventProcessor->setSettingServerHostName(host);
    d->eventProcessor->setSettingServerPort(port);

    qDebug() << "Connecting to" << host << ":" << port;
    d->eventProcessor->run();
}

QSize RemoteDisplayWidget::sizeHint() const {
    Q_D(const RemoteDisplayWidget);
    if (d->desktopSize.isValid()) {
        return d->desktopSize;
    }
    return QWidget::sizeHint();
}

void RemoteDisplayWidget::paintEvent(QPaintEvent *event) {
    Q_D(RemoteDisplayWidget);
    d->eventProcessor->paintDesktopTo(this, event->rect());
}

void RemoteDisplayWidget::mouseMoveEvent(QMouseEvent *event) {
    Q_D(RemoteDisplayWidget);
    d->eventProcessor->sendMouseMoveEvent(event->x(), event->y());
}

void RemoteDisplayWidget::mousePressEvent(QMouseEvent *event) {
    Q_D(RemoteDisplayWidget);
    d->eventProcessor->sendMousePressEvent(event->button(), event->x(), event->y());
}

void RemoteDisplayWidget::mouseReleaseEvent(QMouseEvent *event) {
    Q_D(RemoteDisplayWidget);
    d->eventProcessor->sendMouseReleaseEvent(event->button(), event->x(), event->y());
}
