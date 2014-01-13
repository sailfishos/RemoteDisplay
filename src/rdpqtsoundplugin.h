#ifndef RDPQTSOUNDPLUGIN_H
#define RDPQTSOUNDPLUGIN_H

#include <QPointer>
#include <freerdp/client/rdpsnd.h>

class QIODevice;
class QAudioOutput;
class QAudioFormat;

/**
 * The RdpQtSoundPlugin class lets FreeRDP to play audio through Qt.
 *
 * The class implements rdpsnd's plugin interface which will allow plugging it
 * into FreeRDP. This class acts as wrapper around QAudioOutput.
 */
class RdpQtSoundPlugin {
public:
    static int create(PFREERDP_RDPSND_DEVICE_ENTRY_POINTS pEntryPoints);

    // interface "functions" for FreeRDP
    static void free(rdpsndDevicePlugin* device);
    static void open(rdpsndDevicePlugin* device, AUDIO_FORMAT* format, int latency);
    static void close(rdpsndDevicePlugin* device);
    static BOOL isFormatSupported(rdpsndDevicePlugin* device, AUDIO_FORMAT* format);
    static void setFormat(rdpsndDevicePlugin* device, AUDIO_FORMAT* format, int latency);
    static UINT32 getVolume(rdpsndDevicePlugin* device);
    static void setVolume(rdpsndDevicePlugin* device, UINT32 value);
    static void play(rdpsndDevicePlugin* device, BYTE* data, int size);
    static void start(rdpsndDevicePlugin* device);

private:
    QAudioFormat toQtFormat(AUDIO_FORMAT* in) const;
    void resetAudioOut(const QAudioFormat &format, int avgBytesPerSec);

    RdpQtSoundPlugin();
    ~RdpQtSoundPlugin();

    rdpsndDevicePlugin device;
    QPointer<QAudioOutput> audioOut;
    QPointer<QIODevice> outDevice;
};

#endif // RDPQTSOUNDPLUGIN_H
