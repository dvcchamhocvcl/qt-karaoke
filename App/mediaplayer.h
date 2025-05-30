#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QUrl>
#include <QString>
#include "audiomixer.h"
#include "customaudiooutput.h"

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(float playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)

public:
    explicit MediaPlayer(AudioMixer* audioMixer, QObject *parent = nullptr);
    ~MediaPlayer();

    QUrl source() const;
    void setSource(const QUrl &source);

    float volume() const;
    void setVolume(float volume);

    bool playing() const;

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

    qint64 position() const;
    void setPosition(qint64 position);

    qint64 duration() const;

    float playbackRate() const;
    void setPlaybackRate(float rate);

public slots:
    void play();
    void pause();
    void stop();

signals:
    void sourceChanged();
    void volumeChanged();
    void playingChanged();
    void videoSinkChanged();
    void positionChanged();
    void durationChanged();
    void playbackRateChanged();
    void errorOccurred(const QString &error);

private:
    QMediaPlayer *m_mediaPlayer;
    CustomAudioOutput *m_audioOutput;
    AudioMixer *m_audioMixer;
    QUrl m_source;
    float m_volume;
    bool m_playing;
    QVideoSink *m_videoSink;
    float m_playbackRate;

private slots:
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void handleErrorOccurred(QMediaPlayer::Error error, const QString &errorString);
};

#endif // MEDIAPLAYER_H 