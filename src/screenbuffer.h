#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

class QImage;

/**
 * Common interface for screen buffer classes.
 */
class ScreenBuffer {
public:
    /**
     * Creates and returns an image which contains the whole display of
     * the screen buffer.
     * The returned image can also be null in case of error.
     */
    virtual QImage createImage() const = 0;
};

#endif // SCREENBUFFER_H
