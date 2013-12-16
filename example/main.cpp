#include <QApplication>
#include <QDebug>
#include <remotedisplaywidget.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    RemoteDisplayWidget w;

    auto args = a.arguments();
    if (args.count() < 3) {
        qCritical("Usage: Example.exe <host> <port>");
        return -1;
    }

    w.setDesktopSize(540, 960);
    w.connectToHost(args.at(1), args.at(2).toInt());
    w.show();

    return a.exec();
}

