#ifndef REMOTEDISPLAYWIDGET_P_H
#define REMOTEDISPLAYWIDGET_P_H

#include <QWidget>
#include <QPointer>
#include <QQueue>
#include <QMutex>

class RemoteDisplayWidget;
class QThread;
class EventProcessor;

class RemoteDisplayWidgetPrivate : public QObject {
    Q_OBJECT
public:
    RemoteDisplayWidgetPrivate(RemoteDisplayWidget *q);

    QPointer<QThread> processorThread;
    QPointer<EventProcessor> eventProcessor;
    QSize desktopSize;

    Q_DECLARE_PUBLIC(RemoteDisplayWidget)
    RemoteDisplayWidget* const q_ptr;

private slots:
    void onAboutToConnect();
    void onConnected();
    void onDisconnected();
};

#endif // REMOTEDISPLAYWIDGET_P_H
