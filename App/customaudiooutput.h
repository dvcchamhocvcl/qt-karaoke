#ifndef CUSTOMAUDIOOUTPUT_H
#define CUSTOMAUDIOOUTPUT_H

#include <QAudioSink>
#include <QAudioDevice>
#include <QIODevice>
#include <QAudioFormat>
#include <QBuffer>
#include <QAudioOutput>
#include "audiomixer.h"

// Custom QIODevice to intercept audio data
class AudioInterceptor : public QIODevice
{
    Q_OBJECT
public:
    AudioInterceptor(AudioMixer* mixer, QObject* parent = nullptr);
    
    // Reimplemented methods from QIODevice
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    
private:
    AudioMixer* m_mixer;
};

// Custom audio output that uses our AudioInterceptor
class CustomAudioOutput : public QAudioOutput
{
    Q_OBJECT
public:
    explicit CustomAudioOutput(AudioMixer* mixer, QObject* parent = nullptr);
    ~CustomAudioOutput();
    
    // Get the interceptor, which will be set as the QMediaPlayer's audio device
    QIODevice* interceptor() const { return m_interceptor; }
    
private:
    AudioInterceptor* m_interceptor;
};

#endif // CUSTOMAUDIOOUTPUT_H 
