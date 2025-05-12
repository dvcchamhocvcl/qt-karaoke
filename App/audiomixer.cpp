#include "audiomixer.h"
#include <QDebug>

AudioMixer::AudioMixer(ThreadedAudioManager* audioManager, QObject* parent)
    : QObject(parent), m_audioManager(audioManager)
{
    if (m_audioManager) {
        m_passthrough = m_audioManager->passthrough();
    } else {
        qWarning() << "AudioMixer created without a valid ThreadedAudioManager";
    }
}

AudioMixer::~AudioMixer()
{
    disconnectMediaAudio();
}

void AudioMixer::setInputVolume(float volume)
{
    if (m_inputVolume != volume) {
        m_inputVolume = volume;
        emit inputVolumeChanged();
    }
}

void AudioMixer::setMediaVolume(float volume)
{
    if (m_mediaVolume != volume) {
        m_mediaVolume = volume;
        emit mediaVolumeChanged();
    }
}

void AudioMixer::connectMediaAudio(QIODevice* mediaAudioDevice)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_mediaAudioDevice != mediaAudioDevice) {
        if (m_mediaAudioDevice) {
            disconnectMediaAudio();
        }
        
        m_mediaAudioDevice = mediaAudioDevice;
        
        // Connect to media audio device signals
        if (m_mediaAudioDevice) {
            connect(m_mediaAudioDevice, &QIODevice::readyRead, this, [this]() {
                QMutexLocker locker(&m_mutex);
                
                if (m_mediaAudioDevice && m_passthrough) {
                    // Read data from media audio device
                    QByteArray mediaData = m_mediaAudioDevice->readAll();
                    
                    // Apply volume
                    applyVolume(mediaData, m_mediaVolume);
                    
                    // Write mixed data to passthrough device
                    if (!mediaData.isEmpty()) {
                        m_passthrough->write(mediaData.constData(), mediaData.size());
                    }
                }
            });
        }
    }
}

void AudioMixer::disconnectMediaAudio()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_mediaAudioDevice) {
        disconnect(m_mediaAudioDevice, nullptr, this, nullptr);
        m_mediaAudioDevice = nullptr;
    }
}

void AudioMixer::applyVolume(QByteArray& buffer, float volume)
{
    // Assuming 16-bit PCM audio (Int16)
    if (buffer.size() < 2) {
        return;  // Not enough data
    }
    
    int16_t* samples = reinterpret_cast<int16_t*>(buffer.data());
    int sampleCount = buffer.size() / 2;  // 2 bytes per int16_t sample
    
    for (int i = 0; i < sampleCount; i++) {
        // Apply volume
        float sample = static_cast<float>(samples[i]) * volume;
        
        // Clamp to int16_t range
        if (sample > 32767.0f) {
            sample = 32767.0f;
        } else if (sample < -32768.0f) {
            sample = -32768.0f;
        }
        
        samples[i] = static_cast<int16_t>(sample);
    }
}

QByteArray AudioMixer::mixAudio(const QByteArray& input1, const QByteArray& input2)
{
    // Determine the size of the output buffer (use the smaller buffer size)
    int outputSize = qMin(input1.size(), input2.size());
    outputSize = outputSize - (outputSize % 2);  // Ensure even size for int16_t samples
    
    if (outputSize < 2) {
        return QByteArray();  // Not enough data
    }
    
    QByteArray outputBuffer(outputSize, 0);
    int16_t* outputSamples = reinterpret_cast<int16_t*>(outputBuffer.data());
    const int16_t* input1Samples = reinterpret_cast<const int16_t*>(input1.constData());
    const int16_t* input2Samples = reinterpret_cast<const int16_t*>(input2.constData());
    int sampleCount = outputSize / 2;  // 2 bytes per int16_t sample
    
    for (int i = 0; i < sampleCount; i++) {
        // Mix samples
        float mixedSample = static_cast<float>(input1Samples[i]) + static_cast<float>(input2Samples[i]);
        
        // Prevent clipping
        if (mixedSample > 32767.0f) {
            mixedSample = 32767.0f;
        } else if (mixedSample < -32768.0f) {
            mixedSample = -32768.0f;
        }
        
        outputSamples[i] = static_cast<int16_t>(mixedSample);
    }
    
    return outputBuffer;
}

void AudioMixer::processMediaAudio(const QByteArray& audioData)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_passthrough && !audioData.isEmpty()) {
        // Create a copy of the audio data that we can modify
        QByteArray processedData = audioData;
        
        // Apply volume adjustment
        applyVolume(processedData, m_mediaVolume);
        
        // Write the processed audio data to the passthrough device
        m_passthrough->write(processedData.constData(), processedData.size());
    }
} 