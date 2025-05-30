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

        // Timeline control at the top
        Rectangle {
            id: timelinePanel
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "#404040"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    id: currentTimeText
                    text: formatTime(mediaPlayerBackend.position)
                    color: "white"
                    font.pixelSize: 12
                    Layout.preferredWidth: 50
                }

                Slider {
                    id: timelineSlider
                    Layout.fillWidth: true
                    from: 0
                    to: mediaPlayerBackend.duration
                    value: mediaPlayerBackend.position
                    
                    onMoved: {
                        mediaPlayerBackend.position = value
                    }
                    
                    background: Rectangle {
                        x: timelineSlider.leftPadding
                        y: timelineSlider.topPadding + timelineSlider.availableHeight / 2 - height / 2
                        width: timelineSlider.availableWidth
                        height: 4
                        radius: 2
                        color: "#666666"

                        Rectangle {
                            width: timelineSlider.visualPosition * parent.width
                            height: parent.height
                            color: "#00aaff"
                            radius: 2
                        }
                    }

                    handle: Rectangle {
                        x: timelineSlider.leftPadding + timelineSlider.visualPosition * (timelineSlider.availableWidth - width)
                        y: timelineSlider.topPadding + timelineSlider.availableHeight / 2 - height / 2
                        width: 16
                        height: 16
                        radius: 8
                        color: timelineSlider.pressed ? "#ffffff" : "#00aaff"
                        border.color: "#ffffff"
                        border.width: 1
                    }
                }

                Text {
                    id: durationText
                    text: formatTime(mediaPlayerBackend.duration)
                    color: "white"
                    font.pixelSize: 12
                    Layout.preferredWidth: 50
                }
            }
        }

        // VideoOutput to display the media
        VideoOutput {
            id: videoOutput
            anchors.top: timelinePanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 80
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
            running: false // Don't auto-start
            repeat: false
            onTriggered: {
                console.log("Setting up video sink connection");
                if (videoOutput.videoSink) {
                    mediaPlayerBackend.videoSink = videoOutput.videoSink;
                    mediaPlayerBackend.volume = music_vol_control.value;
                    console.log("Setup complete");
                } else {
                    console.log("VideoOutput sink not ready, retrying...");
                    setupTimer.restart();
                }
            }
        }

        // Setup video connection when component becomes visible
        Component.onCompleted: {
            console.log("MusicControl component loaded");
            setupTimer.start();
        }

        // Cleanup when component is destroyed
        Component.onDestruction: {
            console.log("MusicControl component being destroyed");
            if (mediaPlayerBackend) {
                mediaPlayerBackend.stop();
                mediaPlayerBackend.videoSink = null;
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

            Button {
                id: speedButton
                text: "Speed: " + mediaPlayerBackend.playbackRate + "x"
                onClicked: {
                    // Cycle through speeds: 0.5x -> 1x -> 1.5x -> 0.5x
                    if (mediaPlayerBackend.playbackRate === 0.5) {
                        mediaPlayerBackend.playbackRate = 1.0
                    } else if (mediaPlayerBackend.playbackRate === 1.0) {
                        mediaPlayerBackend.playbackRate = 1.5
                    } else {
                        mediaPlayerBackend.playbackRate = 0.5
                    }
                }
            }

            Button {
                text: "Back"
                onClicked: {
                    startView.state = "Songselect"
                }
            }
        }
    }

    // Function to format time in mm:ss format
    function formatTime(milliseconds) {
        var totalSeconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return minutes.toString().padStart(2, '0') + ":" + seconds.toString().padStart(2, '0')
    }
}
