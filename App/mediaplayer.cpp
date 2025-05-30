#include "mediaplayer.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

MediaPlayer::MediaPlayer(AudioMixer* audioMixer, QObject *parent)
    : QObject(parent)
    , m_audioMixer(audioMixer)
    , m_volume(1.0f)
    , m_playing(false)
    , m_videoSink(nullptr)
{
    // Create the QMediaPlayer instance with a specific render control
    m_mediaPlayer = new QMediaPlayer(this);
    
    // Create our custom audio output and set it up
    m_audioOutput = new CustomAudioOutput(m_audioMixer, this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    
    // Set video output properties - be explicit about video rendering 
    m_mediaPlayer->setVideoOutput(nullptr); // Reset any existing output
    
    // Connect to internal signals
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, 
            this, &MediaPlayer::handleMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &MediaPlayer::handleErrorOccurred);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, [this](QMediaPlayer::PlaybackState state) {
                bool wasPlaying = m_playing;
                m_playing = (state == QMediaPlayer::PlayingState);
                if (wasPlaying != m_playing) {
                    emit playingChanged();
                }
            });
    
    qDebug() << "MediaPlayer created with software rendering";
}

MediaPlayer::~MediaPlayer()
{
    // Stop playback
    if (m_mediaPlayer) {
        m_mediaPlayer->stop();
    }
}

QUrl MediaPlayer::source() const
{
    return m_source;
}

void MediaPlayer::setSource(const QUrl &source)
{
    if (m_source != source) {
        m_source = source;
        m_mediaPlayer->setSource(source);
        emit sourceChanged();
    }
}

float MediaPlayer::volume() const
{
    return m_volume;
}

void MediaPlayer::setVolume(float volume)
{
    if (m_volume != volume) {
        m_volume = volume;
        
        // Apply volume to our audio output
        m_audioOutput->setVolume(volume);
        
        // If we have a mixer, tell it about the volume change
        if (m_audioMixer) {
            m_audioMixer->setMediaVolume(volume);
        }
        
        emit volumeChanged();
    }
}

bool MediaPlayer::playing() const
{
    return m_playing;
}

QVideoSink* MediaPlayer::videoSink() const
{
    return m_videoSink;
}

void MediaPlayer::setVideoSink(QVideoSink* sink)
{
    if (m_videoSink != sink) {
        // Cleanup previous connection
        if (m_videoSink) {
            qDebug() << "Cleaning up previous video sink";
            m_mediaPlayer->setVideoSink(nullptr);
        }
        
        m_videoSink = sink;
        
        if (sink) {
            qDebug() << "Setting new video sink on media player";
            
            // Stop playback before changing video sink
            bool wasPlaying = m_playing;
            if (wasPlaying) {
                m_mediaPlayer->pause();
            }
            
            // Set the new video sink
            m_mediaPlayer->setVideoSink(sink);
            
            // Check if we have a source loaded and reload if necessary
            if (!m_source.isEmpty()) {
                qDebug() << "Reloading source after video sink change";
                QUrl currentSource = m_source;
                m_mediaPlayer->setSource(QUrl()); // Clear first
                m_mediaPlayer->setSource(currentSource); // Reload
            }
            
            // Resume playback if it was playing before
            if (wasPlaying) {
                QTimer::singleShot(100, this, [this]() {
                    m_mediaPlayer->play();
                });
            }
        } else {
            qDebug() << "Video sink set to null - clearing media player video output";
            m_mediaPlayer->setVideoSink(nullptr);
        }
        
        emit videoSinkChanged();
    }
}

void MediaPlayer::play()
{
    // Check if we have a valid source
    if (m_source.isEmpty()) {
        qWarning() << "Cannot play - no media source set";
        emit errorOccurred("No media source set");
        return;
    }
    
    qDebug() << "Playing media: " << m_source.toString();
    
    // Check if video sink is connected
    if (m_videoSink) {
        qDebug() << "Video sink is connected";
        
        // Ensure video sink is correctly set before playing
        if (m_mediaPlayer->hasVideo()) {
            qDebug() << "Media has video content, ensuring video sink is set";
            m_mediaPlayer->setVideoSink(m_videoSink);
        } else {
            qDebug() << "Media doesn't appear to have video content yet";
        }
    } else {
        qWarning() << "Playing with no video sink connected";
    }
    
    // Double-check the media status before playing
    if (m_mediaPlayer->mediaStatus() == QMediaPlayer::LoadedMedia || 
        m_mediaPlayer->mediaStatus() == QMediaPlayer::BufferedMedia) {
        qDebug() << "Media is ready to play";
    } else {
        qDebug() << "Media might not be fully loaded, current status: " 
                 << m_mediaPlayer->mediaStatus();
        
        // If not loaded, try to reload
        QUrl currentSource = m_source;
        m_mediaPlayer->setSource(QUrl());  // Clear it first
        m_mediaPlayer->setSource(currentSource);  // Set it again
    }
    
    // Start playback
    m_mediaPlayer->play();
    
    // Log video and audio state after play
    qDebug() << "After play() call - hasVideo: " << m_mediaPlayer->hasVideo()
             << ", position: " << m_mediaPlayer->position()
             << ", duration: " << m_mediaPlayer->duration();
}

void MediaPlayer::pause()
{
    m_mediaPlayer->pause();
}

void MediaPlayer::stop()
{
    m_mediaPlayer->stop();
}

void MediaPlayer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // Log all media status changes
    QString statusStr;
    switch (status) {
        case QMediaPlayer::NoMedia: statusStr = "NoMedia"; break;
        case QMediaPlayer::LoadingMedia: statusStr = "LoadingMedia"; break;
        case QMediaPlayer::LoadedMedia: statusStr = "LoadedMedia"; break;
        case QMediaPlayer::StalledMedia: statusStr = "StalledMedia"; break;
        case QMediaPlayer::BufferingMedia: statusStr = "BufferingMedia"; break;
        case QMediaPlayer::BufferedMedia: statusStr = "BufferedMedia"; break;
        case QMediaPlayer::EndOfMedia: statusStr = "EndOfMedia"; break;
        case QMediaPlayer::InvalidMedia: statusStr = "InvalidMedia"; break;
        default: statusStr = "Unknown"; break;
    }
    
    qDebug() << "Media status changed: " << statusStr;
    
    if (status == QMediaPlayer::LoadedMedia) {
        // Media is loaded and ready to play
        qDebug() << "Media loaded successfully: " << m_source.toString();
        
        // Check video aspects
        qDebug() << "Video available: " << m_mediaPlayer->hasVideo();
        qDebug() << "Video sink connected: " << (m_videoSink != nullptr);
        
        // If the video sink wasn't connected yet, try again
        if (m_mediaPlayer->hasVideo() && m_videoSink) {
            qDebug() << "Re-applying video sink to ensure connection";
            m_mediaPlayer->setVideoSink(m_videoSink);
        }
    }
    else if (status == QMediaPlayer::InvalidMedia) {
        qWarning() << "Invalid media: " << m_mediaPlayer->errorString();
    }
}

void MediaPlayer::handleErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error);
    emit errorOccurred(errorString);
    qWarning() << "Media player error: " << errorString;
}

