#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIODevice>
#include <QUrl>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include "autogen/environment.h"
#include "audiopassthrough.h"
#include "audiomixer.h"
#include "mediaplayer.h"

int main(int argc, char *argv[])
{
    // Set appropriate rendering backend for Raspberry Pi
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL); // or simply omit the call
    
    set_qt_environment();
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Create the threaded audio manager
    ThreadedAudioManager* audioManager = new ThreadedAudioManager(&app);

    // Create the audio mixer
    AudioMixer* audioMixer = new AudioMixer(audioManager, &app);
    
    // Create the media player
    MediaPlayer* mediaPlayer = new MediaPlayer(audioMixer, &app);

    // Start the audio threads
    audioManager->start();

    // Register the audio manager and mixer if needed in QML
    engine.rootContext()->setContextProperty("audioManager", audioManager);
    engine.rootContext()->setContextProperty("audioMixer", audioMixer);
    engine.rootContext()->setContextProperty("mediaPlayerBackend", mediaPlayer);

    const QUrl url(mainQmlFile); // Assuming mainQmlFile is defined in environment.h
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
        );

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML root object!";
        return -1;
    }

    return app.exec();
}
