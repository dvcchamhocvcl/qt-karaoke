#include "audiopassthrough.h"
#include <QMediaDevices>
#include <QDebug>


//---------- AudioPassthrough Implementation ----------

AudioPassthrough::AudioPassthrough(QObject *parent) : QIODevice(parent) {
    open(QIODevice::ReadWrite);
}

qint64 AudioPassthrough::readData(char *data, qint64 maxSize) {
    QMutexLocker locker(&m_mutex);

    qint64 bytesToRead = qMin(maxSize, qint64(m_buffer.size()));
    if (bytesToRead > 0) {
        // Copy data from our buffer to the output buffer
        memcpy(data, m_buffer.constData(), bytesToRead);

        // Remove read data from our buffer
        m_buffer.remove(0, bytesToRead);
    }
    return bytesToRead;
}

qint64 AudioPassthrough::writeData(const char *data, qint64 maxSize) {
    QMutexLocker locker(&m_mutex);
    if (m_buffer.size() + maxSize > m_maxBufferSize) {
        // Buffer is too large, drop oldest data
        int bytesToDrop = (m_buffer.size() + maxSize) - m_maxBufferSize;
        m_buffer.remove(0, bytesToDrop);
    }
    
    m_buffer.append(data, maxSize);
    // Signal that data is available to be read
    emit readyRead();

    // Emit signal that new data was written

    return maxSize;
}

//---------- AudioInputThread Implementation ----------

AudioInputThread::AudioInputThread(AudioPassthrough* passthrough, QObject* parent)
    : QThread(parent), m_passthrough(passthrough) {

    // Set default format
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

AudioInputThread::~AudioInputThread() {
    stop();
    wait(); // Wait for thread to finish
}

void AudioInputThread::setFormat(const QAudioFormat& format) {
    if (!isRunning()) {
        m_format = format;
    } else {
        qWarning() << "Cannot change format while thread is running";
    }
}

void AudioInputThread::run() {
    // Create audio input device in this thread
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isFormatSupported(m_format)) {
        qWarning() << "Default input format not supported, trying to use nearest";
        m_format = inputDevice.preferredFormat();
    }

    // Create audio source
    m_audioSource = new QAudioSource(inputDevice, m_format);

    // Start capturing
    m_running = true;
    m_audioSource->start(m_passthrough);

    qDebug() << "Audio input thread started";

    // Run event loop for this thread
    exec();

    // Clean up when event loop exits
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
    }

    qDebug() << "Audio input thread stopped";
}

void AudioInputThread::stop() {
    if (isRunning()) {
        m_running = false;
        quit(); // Tell event loop to quit
    }
}

//---------- AudioOutputThread Implementation ----------

AudioOutputThread::AudioOutputThread(AudioPassthrough* passthrough, QObject* parent)
    : QThread(parent), m_passthrough(passthrough) {

    // Set default format
    m_format.setSampleRate(44100);
    m_format.setChannelCount(2);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

AudioOutputThread::~AudioOutputThread() {
    stop();
    wait(); // Wait for thread to finish
}

void AudioOutputThread::setFormat(const QAudioFormat& format) {
    if (!isRunning()) {
        m_format = format;
    } else {
        qWarning() << "Cannot change format while thread is running";
    }
}

void AudioOutputThread::run() {
    // Create audio output device in this thread
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (!outputDevice.isFormatSupported(m_format)) {
        qWarning() << "Default output format not supported";
    }

    // Create audio sink
    m_audioSink = new QAudioSink(outputDevice, m_format);

    // Start playback
    m_running = true;
    m_audioSink->start(m_passthrough);

    qDebug() << "Audio output thread started";

    // Run event loop for this thread
    exec();

    // Clean up when event loop exits
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    qDebug() << "Audio output thread stopped";
}

void AudioOutputThread::stop() {
    if (isRunning()) {
        m_running = false;
        quit(); // Tell event loop to quit
    }
}

//---------- ThreadedAudioManager Implementation ----------

ThreadedAudioManager::ThreadedAudioManager(QObject* parent) : QObject(parent) {
    // Create the passthrough device
    m_passthrough = new AudioPassthrough(this);

    // Set default format
    m_inputformat.setSampleRate(44100);
    m_inputformat.setChannelCount(1);
    m_inputformat.setSampleFormat(QAudioFormat::Int16);
    m_outputformat.setSampleRate(44100);
    m_outputformat.setChannelCount(1);
    m_outputformat.setSampleFormat(QAudioFormat::Int16);

    // Create threads
    m_inputThread = new AudioInputThread(m_passthrough, this);
    m_outputThread = new AudioOutputThread(m_passthrough, this);

    // Set formats
    m_inputThread->setFormat(m_inputformat);
    m_outputThread->setFormat(m_outputformat);
}

ThreadedAudioManager::~ThreadedAudioManager() {
    stop();
}

void ThreadedAudioManager::start() {
    // Start threads
    m_inputThread->start(QThread::TimeCriticalPriority);
    m_outputThread->start(QThread::TimeCriticalPriority);

    qDebug() << "Audio manager started";
}

void ThreadedAudioManager::stop() {
    // Stop threads
    if (m_inputThread) {
        m_inputThread->stop();
    }

    if (m_outputThread) {
        m_outputThread->stop();
    }

    qDebug() << "Audio manager stopped";
}
