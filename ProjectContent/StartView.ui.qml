
/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import Project

Rectangle {
    id: startView
    width: Constants.width
    height: Constants.height
    visible: true
    color: "#f6f5f4"
    state: "Songselect"

    Loader {
        id: songselect
        visible: false
        anchors.fill: parent
        source: "SongSelectScreen.ui.qml"
        active: false
    }

    Loader {
        id: mediaplayer
        visible: false
        anchors.fill: parent
        source: "MediaPlayer.ui.qml"
        active: false
    }

    Loader {
        id: musiccontrol
        visible: false
        anchors.fill: parent
        source: "MusicControl.ui.qml"
        active: false
    }
    states: [
        State {
            name: "Songselect"

            PropertyChanges {
                target: songselect
                x: 0
                y: 0
                width: Constants.width
                height: Constants.height
                visible: true
                active: true
            }
        },
        State {
            name: "Mediaplayer"

            PropertyChanges {
                target: mediaplayer
                visible: false
                active: false
            }

            PropertyChanges {
                target: musiccontrol
                visible: true
                active: true
            }
        },

        State {
            name: "Musiccontrol"

            PropertyChanges {
                target: songselect
                x: 0
                y: 0
                width: Constants.width
                height: Constants.height
                visible: false
                source: "SongSelectScreen.ui.qml"
                active: false
            }

            PropertyChanges {
                target: musiccontrol
                visible: true
                active: true
            }
        }
    ]
}
