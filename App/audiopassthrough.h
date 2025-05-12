#ifndef AUDIOPASSTHROUGH_H
#define AUDIOPASSTHROUGH_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioFormat>
#include <QAudioDevice>

// Shared QIODevice that can be safely accessed from multiple threads
class AudioPassthrough : public QIODevice {
    Q_OBJECT

private:
    QByteArray m_buffer;
    QMutex m_mutex;
    int m_maxBufferSize = 1024 * 1024; // 1MB max buffer size

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

public:
    explicit AudioPassthrough(QObject *parent = nullptr);


};

// Thread for handling audio input
class AudioInputThread : public QThread {
    Q_OBJECT

private:
    QAudioSource* m_audioSource = nullptr;
    AudioPassthrough* m_passthrough = nullptr;
    QAudioFormat m_format;
    bool m_running = false;

public:
    explicit AudioInputThread(AudioPassthrough* passthrough, QObject* parent = nullptr);
    ~AudioInputThread();

    void setFormat(const QAudioFormat& format);

protected:
    void run() override;

public slots:
    void stop();
};

// Thread for handling audio output
class AudioOutputThread : public QThread {
    Q_OBJECT

private:
    QAudioSink* m_audioSink = nullptr;
    AudioPassthrough* m_passthrough = nullptr;
    QAudioFormat m_format;
    bool m_running = false;

public:
    explicit AudioOutputThread(AudioPassthrough* passthrough, QObject* parent = nullptr);
    ~AudioOutputThread();

    void setFormat(const QAudioFormat& format);

protected:
    void run() override;

public slots:
    void stop();
};

// Manager class that ties everything together
class ThreadedAudioManager : public QObject {
    Q_OBJECT

private:
    AudioPassthrough* m_passthrough = nullptr;
    AudioInputThread* m_inputThread = nullptr;
    AudioOutputThread* m_outputThread = nullptr;
    QAudioFormat m_inputformat;
    QAudioFormat m_outputformat;

public:
    explicit ThreadedAudioManager(QObject* parent = nullptr);
    ~ThreadedAudioManager();

    // Return the passthrough device for external access if needed
    AudioPassthrough* passthrough() const { return m_passthrough; }

public slots:
    void start();
    void stop();
};

#endif // THREADED_AUDIOPASSTHROUGH_H
