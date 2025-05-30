/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtMultimedia
import Project
import QtQuick.Studio.Components 1.0
import QtQuick.Layouts 1.15

Rectangle {
    id: musiccontrol
    width: Constants.width
    height: Constants.height
    color: "#f6f5f4"

    // Left panel for audio controls
    Rectangle {
        id: controlPanel
        width: parent.width * 0.25
        height: parent.height
        color: "#f6f5f4"

        Slider {
            id: volumeSlider
            x: 36
            y: 173
            value: 0.75
            orientation: Qt.Vertical
            scale: 1.9
            rotation: 0
            from: 0.0
            to: 2.0
            stepSize: 0.1

            onValueChanged: {
                // Call the C++ method to adjust microphone volume
                audioMixer.inputVolume = value
            }

            Text {
                text: "Mic Volume"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -30
            }
        }

        Slider {
            id: music_vol_control
            x: 145
            y: 173
            value: 0.75
            scale: 1.9
            rotation: 0
            orientation: Qt.Vertical
            from: 0.0
            to: 1.0
            stepSize: 0.1

            onValueChanged: {
                // Adjust the media player volume
                mediaPlayerBackend.volume = value
            }

            Text {
                text: "Music Volume"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -30
            }
        }


    }

    // Right panel for media player
    Rectangle {
        id: mediaPanel
        anchors.left: controlPanel.right
        width: parent.width * 0.75
        height: parent.height
        color: "#2c2c2c"

        // VideoOutput to display the media
        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectFit
            
            // Make sure this is visible
            visible: true
            z: 10 // Ensure it's on top
            
            // Make the background color obvious to debug visibility issues
            Rectangle {
                anchors.fill: parent
                z: -1
                color: "blue"
                opacity: 0.2
                visible: mediaPlayerBackend.playing
            }
        }

        // Connect QML VideoOutput to our C++ MediaPlayer with a slight delay to ensure UI is ready
        Timer {
            id: setupTimer
            interval: 500 // half second delay
            running: true
            repeat: false
            onTriggered: {
                console.log("Setting up video sink connection");
                mediaPlayerBackend.videoSink = videoOutput.videoSink;
                mediaPlayerBackend.source = "/home/karaoke/nuathapki.mp4";
                mediaPlayerBackend.volume = music_vol_control.value;
                console.log("Setup complete");
            }
        }

        // Update the media player volume when slider changes
        Connections {
            target: music_vol_control
            function onValueChanged() {
                mediaPlayerBackend.volume = music_vol_control.value
            }
        }

        // Show a placeholder when no video is playing
        Rectangle {
            id: placeholderBackground
            anchors.fill: parent
            color: "#1a1a1a"
            visible: !mediaPlayerBackend.playing
            
            Text {
                anchors.centerIn: parent
                text: "Click Play to start video"
                color: "white"
                font.pixelSize: 20
            }
        }

        RowLayout {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 20
            spacing: 10

            Button {
                text: mediaPlayerBackend.playing ? "Pause" : "Play"
                onClicked: {
                    if (mediaPlayerBackend.playing)
                        mediaPlayerBackend.pause()
                    else
                        mediaPlayerBackend.play()
                }
            }

            Button {
                text: "Stop"
                onClicked: mediaPlayerBackend.stop()
            }

            ComboBox {
                id: mediaSelector
                model: ["Video 1", "Video 2", "Video 3"] // Replace with your available media
                onCurrentTextChanged: {
                    // Set media source based on selection
                    // This should be replaced with actual paths to your media files
                    switch(currentText) {
                        case "Video 1":
                            mediaPlayerBackend.source = "/home/karaoke/nuathapki.mp4"
                            break
                        case "Video 2":
                            mediaPlayerBackend.source = "/home/karaoke/codontrensofa.mp4"
                            break
                        case "Video 3":
                            mediaPlayerBackend.source = "file:///path/to/video3.mp4"
                            break
                    }
                }
            }
        }
    }
}
