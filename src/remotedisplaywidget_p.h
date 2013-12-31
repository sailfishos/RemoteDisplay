#ifndef REMOTEDISPLAYWIDGET_P_H
#define REMOTEDISPLAYWIDGET_P_H

#include <QWidget>
#include <QPointer>
#include <QQueue>
#include <QMutex>
#include <QTransform>

class RemoteDisplayWidget;
class QThread;
class FreeRdpClient;
class Cursor;

class RemoteDisplayWidgetPrivate : public QObject {
    Q_OBJECT
public:
    RemoteDisplayWidgetPrivate(RemoteDisplayWidget *q);

    QPoint mapToRemoteDesktop(const QPoint &local) const;

    QPointer<QThread> processorThread;
    QPointer<FreeRdpClient> eventProcessor;
    QSize desktopSize;
    QRect scaledDesktopRect;
    QTransform scaledDesktopMapper;

    Q_DECLARE_PUBLIC(RemoteDisplayWidget)
    RemoteDisplayWidget* const q_ptr;

private slots:
    void onAboutToConnect();
    void onConnected();
    void onDisconnected();
    void onCursorChanged(const Cursor &cursor);
};

#endif // REMOTEDISPLAYWIDGET_P_H
