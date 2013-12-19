#ifndef MYCONTEXT_H
#define MYCONTEXT_H

class FreeRdpClient;
class CursorChangeNotifier;

#include <QImage>
#include <freerdp/freerdp.h>

struct MyContext {
    MyContext();
    rdpContext freeRdpContext;
    FreeRdpClient *self;
    CursorChangeNotifier *cursorChangeNotifier;
};

MyContext* getMyContext(rdpContext* context);
MyContext* getMyContext(freerdp* instance);

QImage::Format bppToImageFormat(int bpp);

#endif // MYCONTEXT_H
