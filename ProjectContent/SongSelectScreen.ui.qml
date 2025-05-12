

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

    ColumnLayout {
        id: columnLayout
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 200
        anchors.topMargin: 200
        ComboBox {
            id: comboBox
        }

        Button {
            id: button
            text: qsTr("Button")

            Connections {
                target: button
                function onClicked() {
                    startView.state = "Mediaplayer"
                }
            }
        }
    }
}
