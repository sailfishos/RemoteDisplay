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

    void setDesktopSize(quint16 width, quint16 height);
    void connectToHost(const QString &host, quint16 port);

    virtual QSize sizeHint() const;

signals:
    /**
     * This signal is emitted when connecting to host fails or if already
     * established connection breaks.
     */
    void disconnected();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

private:
    Q_DECLARE_PRIVATE(RemoteDisplayWidget)
    RemoteDisplayWidgetPrivate* const d_ptr;
};

#endif // REMOTEDISPLAYWIDGET_H
