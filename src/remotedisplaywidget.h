#ifndef REMOTEDISPLAYWIDGET_H
#define REMOTEDISPLAYWIDGET_H

#include <QWidget>
#include "global.h"

class RemoteDisplayWidgetPrivate;

class REMOTEDISPLAYSHARED_EXPORT RemoteDisplayWidget : public QWidget {
    Q_OBJECT
public:
    RemoteDisplayWidget(QWidget *parent = 0);
    ~RemoteDisplayWidget();

    void connectToHost(const QString &host, quint16 port);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(RemoteDisplayWidget)
    RemoteDisplayWidgetPrivate* const d_ptr;
};

#endif // REMOTEDISPLAYWIDGET_H
