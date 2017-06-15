import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import FuckTheSystem 0.0


Item {
    id: root

    property real font_size: height*.032
    property real side_size: height*.1
    property real margins: side_size*.1

    Rectangle {
        id: navigation_block
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progress_bar.top
        width: side_size*3 + margins*4
        height: side_size + margins*2
        // color: "green"
        color: "transparent"

        Button2 {
            id: b_left
            width: side_size
            height: side_size
            anchors.top: parent.top
            anchors.right: b_enter.left

            anchors.margins: margins
            font_pixel_size: font_size
            text: "<<"
            onClicked: messenger.keyPressed(KeyCode.Left)
        }

        Button2 {
            id: b_right
            width: side_size
            height: side_size
            anchors.top: parent.top
            anchors.left: b_enter.right
            anchors.margins: margins
            font_pixel_size: font_size
            text: ">>"
            onClicked: messenger.keyPressed(KeyCode.Right)
        }

        Button2 {
            id: b_enter
            width: side_size
            height: side_size
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            font_pixel_size: font_size
            text: "|| / >"
            onClicked: messenger.keyPressed(KeyCode.Menu)
        }
    }

    PlayerState {
        id: progress_bar
        width: root.width*.75
        height: side_size
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height*.6
    }
}
