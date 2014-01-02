#ifndef BITMAPRECTANGLESINK_H
#define BITMAPRECTANGLESINK_H

class QRect;
class QByteArray;

/**
 * The BitmapRectangleSink interface provides a sink where bitmap rectangle
 * updates received from remote host can be fed into.
 */
class BitmapRectangleSink {
public:
    /**
     * This method adds given bitmap rectangle to the sink.
     */
    virtual void addRectangle(const QRect &rect, const QByteArray &data) = 0;
};

#endif // BITMAPRECTANGLESINK_H
