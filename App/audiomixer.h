#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <QObject>
#include <QAudioSink>
#include <QAudioSource>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QIODevice>
#include <QByteArray>
#include <QMutex>
#include "audiopassthrough.h"

class AudioMixer : public QObject {
    Q_OBJECT
    Q_PROPERTY(float inputVolume READ inputVolume WRITE setInputVolume NOTIFY inputVolumeChanged)
    Q_PROPERTY(float mediaVolume READ mediaVolume WRITE setMediaVolume NOTIFY mediaVolumeChanged)

public:
    explicit AudioMixer(ThreadedAudioManager* audioManager, QObject* parent = nullptr);
    ~AudioMixer();

    // Get and set input audio volume
    float inputVolume() const { return m_inputVolume; }
    Q_INVOKABLE void setInputVolume(float volume);

    // Get and set media audio volume
    float mediaVolume() const { return m_mediaVolume; }
    Q_INVOKABLE void setMediaVolume(float volume);

    // Connect a media player's audio buffer to the mixer
    void connectMediaAudio(QIODevice* mediaAudioDevice);
    void disconnectMediaAudio();
    
    // Process audio data from the media player
    void processMediaAudio(const QByteArray& audioData);

signals:
    void inputVolumeChanged();
    void mediaVolumeChanged();

private:
    ThreadedAudioManager* m_audioManager;
    AudioPassthrough* m_passthrough;
    QIODevice* m_mediaAudioDevice = nullptr;
    
    float m_inputVolume = 1.0f;
    float m_mediaVolume = 1.0f;
    
    QMutex m_mutex;
    
    // Apply volume to audio data
    void applyVolume(QByteArray& buffer, float volume);
    
    // Mix two audio streams together
    QByteArray mixAudio(const QByteArray& input1, const QByteArray& input2);
};

#endif // AUDIOMIXER_H 