#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include <QWidget>

typedef struct rdp_freerdp freerdp;

class EventProcessor : public QObject {
    Q_OBJECT
public:
    EventProcessor(freerdp* instance);
    ~EventProcessor();

    void requestStop();

public slots:
    void run();

private:
    freerdp* freeRdpInstance;
    bool stop;
};

#endif // EVENTPROCESSOR_H
