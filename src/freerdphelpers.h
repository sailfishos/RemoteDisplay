#ifndef MYCONTEXT_H
#define MYCONTEXT_H

class FreeRdpClient;

#include <QImage>
#include <freerdp/freerdp.h>

struct MyContext {
    MyContext();
    rdpContext freeRdpContext;
    FreeRdpClient *self;
};

MyContext* getMyContext(rdpContext* context);
MyContext* getMyContext(freerdp* instance);

QImage::Format bppToImageFormat(int bpp);

#endif // MYCONTEXT_H
