#include "customaudiooutput.h"
#include <QDebug>

// --- AudioInterceptor Implementation ---

AudioInterceptor::AudioInterceptor(AudioMixer* mixer, QObject* parent)
    : QIODevice(parent), m_mixer(mixer)
{
    // Open the device for read and write
    open(QIODevice::ReadWrite);
}

qint64 AudioInterceptor::readData(char *data, qint64 maxSize)
{
    // This is called when the audio system needs data to play
    // In our case, we don't need to implement this since we're focusing on
    // intercepting the write operations from the media player
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return 0;
}

qint64 AudioInterceptor::writeData(const char *data, qint64 maxSize)
{
    // This is where we intercept audio data from the media player
    if (m_mixer && maxSize > 0) {
        // Create a QByteArray from the data
        QByteArray audioData(data, maxSize);
        
        // Send it to our mixer
        m_mixer->processMediaAudio(audioData);
    }
    
    // Return maxSize to indicate we've processed all the data
    return maxSize;
}

// --- CustomAudioOutput Implementation ---

CustomAudioOutput::CustomAudioOutput(AudioMixer* mixer, QObject* parent)
    : QAudioOutput(parent)
{
    // Create the interceptor
    m_interceptor = new AudioInterceptor(mixer, this);
}

CustomAudioOutput::~CustomAudioOutput()
{
    // Clean up if needed
    if (m_interceptor) {
        m_interceptor->close();
    }
} 