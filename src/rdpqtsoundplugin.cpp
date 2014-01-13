#include "rdpqtsoundplugin.h"
#include <QAudioOutput>
#include <QAudioFormat>
#include <QDebug>

#define SELF(ARG) ((RdpQtSoundPlugin*)ARG)

int RdpQtSoundPlugin::create(PFREERDP_RDPSND_DEVICE_ENTRY_POINTS pEntryPoints) {
    auto plugin = new RdpQtSoundPlugin;

    plugin->device.Open = open;
    plugin->device.FormatSupported = isFormatSupported;
    plugin->device.SetFormat = setFormat;
    plugin->device.GetVolume = getVolume;
    plugin->device.SetVolume = setVolume;
    plugin->device.Play = play;
    plugin->device.Start = start;
    plugin->device.Close = close;
    plugin->device.Free = free;

    pEntryPoints->pRegisterRdpsndDevice(pEntryPoints->rdpsnd, (rdpsndDevicePlugin*)plugin);

    return 0;
}

void RdpQtSoundPlugin::free(rdpsndDevicePlugin *device) {
    delete SELF(device);
}

void RdpQtSoundPlugin::open(rdpsndDevicePlugin *device, AUDIO_FORMAT *format, int latency) {
    auto qFormat = SELF(device)->toQtFormat(format);
    SELF(device)->resetAudioOut(qFormat, format->nAvgBytesPerSec);
    SELF(device)->outDevice = SELF(device)->audioOut->start();
}

void RdpQtSoundPlugin::close(rdpsndDevicePlugin *device) {
    SELF(device)->audioOut->stop();
}

BOOL RdpQtSoundPlugin::isFormatSupported(rdpsndDevicePlugin *device, AUDIO_FORMAT *format) {
    return SELF(device)->toQtFormat(format).isValid();
}

void RdpQtSoundPlugin::setFormat(rdpsndDevicePlugin *device, AUDIO_FORMAT *format, int latency) {
    auto qFormat = SELF(device)->toQtFormat(format);
    SELF(device)->resetAudioOut(qFormat, format->nAvgBytesPerSec);
    // TODO: What to do with 'latency'?
}

UINT32 RdpQtSoundPlugin::getVolume(rdpsndDevicePlugin *device) {
    // QAudioOutput in Qt5 provides get/set for volume, but not for Qt4
    qDebug() << "getVolume(): Not implemented";
    return 0;
}

void RdpQtSoundPlugin::setVolume(rdpsndDevicePlugin *device, UINT32 value) {
    // QAudioOutput in Qt5 provides get/set for volume, but not for Qt4
    qDebug() << "setVolume(): Not implemented";
}

void RdpQtSoundPlugin::play(rdpsndDevicePlugin *device, BYTE *data, int size) {
    if (SELF(device)->outDevice) {
        auto wrote = SELF(device)->outDevice->write((char*)data, size);
        if (wrote < size) {
            qWarning() << "Sound buffer full. Failed to write" << (size - wrote) << "bytes.";
        }
    }
}

void RdpQtSoundPlugin::start(rdpsndDevicePlugin *device) {
    SELF(device)->audioOut->reset();
}

QAudioFormat RdpQtSoundPlugin::toQtFormat(AUDIO_FORMAT *in) const {
    if (in == NULL) {
        return QAudioFormat();
    }

    QAudioFormat out;
    out.setCodec("audio/pcm");
    out.setSampleRate(in->nSamplesPerSec);
    out.setChannelCount(in->nChannels);
    out.setSampleSize(in->wBitsPerSample);
    // TODO: what is correct value for sample type?
    out.setSampleType(QAudioFormat::UnSignedInt);
    return out;
}

void RdpQtSoundPlugin::resetAudioOut(const QAudioFormat &format, int avgBytesPerSec) {
    delete audioOut;
    audioOut = new QAudioOutput(format);
    // buffer size worth of 10 seconds should be enough
    audioOut->setBufferSize(avgBytesPerSec * 10);
}

RdpQtSoundPlugin::RdpQtSoundPlugin() {
    memset(&device, 0, sizeof(device));
    resetAudioOut(QAudioFormat(), 0);
}

RdpQtSoundPlugin::~RdpQtSoundPlugin() {
    delete audioOut;
}
