import QtQuick 2.7
import QtQuick.Window 2.2

import "qrc:/qml"


Rectangle {
    visible: true
    color: "transparent"
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight


    RecordState {
        anchors.horizontalCenter: parent.horizontalCenter
        opacity:0
        state_visible: true
        y: Screen.height - height*1.5
    }
}

