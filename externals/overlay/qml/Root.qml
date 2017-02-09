import QtQuick 2.7
import QtQuick.Window 2.2

import "qrc:/qml"


Rectangle {
    visible: true

    color: "transparent"

    width: Screen.width
    height: Screen.height

    Settings {
        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent

        opacity: 0
    }

    RecordState {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height - height*1.5

        opacity: 0
    }
}

