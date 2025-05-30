/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import Project 1.0
import QtQuick.Layouts

Rectangle {
    id: songselect
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor

    Text {
        id: titleText
        text: "Select a Song"
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 50
        font.pixelSize: 32
        font.bold: true
        color: "#333333"
    }

    GridLayout {
        id: videoGrid
        anchors.centerIn: parent
        columns: 3
        rowSpacing: 30
        columnSpacing: 30

        // Video 1: nuathapki.mp4
        Rectangle {
            id: video1Container
            width: 200
            height: 180
            color: "#f0f0f0"
            border.color: "#cccccc"
            border.width: 2
            radius: 10

            Column {
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    id: thumbnail1
                    width: 160
                    height: 120
                    color: "#333333"
                    radius: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "🎵"
                        font.pixelSize: 40
                        color: "white"
                    }
                }

                Text {
                    text: "Nua Thap Ki"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 14
                    font.bold: true
                    color: "#333333"
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mediaPlayerBackend.source = "/home/karaoke/nuathapki.mp4"
                    startView.state = "Mediaplayer"
                }
            }
        }

        // Video 2: codontrensofa.mp4
        Rectangle {
            id: video2Container
            width: 200
            height: 180
            color: "#f0f0f0"
            border.color: "#cccccc"
            border.width: 2
            radius: 10

            Column {
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    id: thumbnail2
                    width: 160
                    height: 120
                    color: "#333333"
                    radius: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "🎵"
                        font.pixelSize: 40
                        color: "white"
                    }
                }

                Text {
                    text: "Co Don Tren Sofa"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 14
                    font.bold: true
                    color: "#333333"
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mediaPlayerBackend.source = "/home/karaoke/codontrensofa.mp4"
                    startView.state = "Mediaplayer"
                }
            }
        }

        // Video 3: taivisao.mp4
        Rectangle {
            id: video3Container
            width: 200
            height: 180
            color: "#f0f0f0"
            border.color: "#cccccc"
            border.width: 2
            radius: 10

            Column {
                anchors.centerIn: parent
                spacing: 10

                Rectangle {
                    id: thumbnail3
                    width: 160
                    height: 120
                    color: "#333333"
                    radius: 8
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "🎵"
                        font.pixelSize: 40
                        color: "white"
                    }
                }

                Text {
                    text: "Tai Vi Sao"
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: 14
                    font.bold: true
                    color: "#333333"
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mediaPlayerBackend.source = "/home/karaoke/taivisao.mp4"
                    startView.state = "Mediaplayer"
                }
            }
        }
    }
}
