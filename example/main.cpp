#include <QApplication>
#include <QDebug>
#include <remotedisplaywidget.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    RemoteDisplayWidget w;

    auto args = a.arguments();
    if (args.count() < 5) {
        qCritical("Usage: Example.exe <host> <port> <width> <height>");
        return -1;
    }

    auto host = args.at(1);
    auto port = args.at(2).toInt();
    auto width = args.at(3).toInt();
    auto height = args.at(4).toInt();

    w.resize(width, height);
    w.setDesktopSize(width, height);
    w.connectToHost(host, port);
    w.show();

    QObject::connect(&w, SIGNAL(disconnected()), &a, SLOT(quit()));

    return a.exec();
}

