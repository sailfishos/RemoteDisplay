#include <QApplication>
#include <remotedisplaywidget.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    RemoteDisplayWidget w;
    w.show();

    return a.exec();
}

