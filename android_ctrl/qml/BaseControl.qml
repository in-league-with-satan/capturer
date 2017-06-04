import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import FuckTheSystem 0.0

Item {
    id: root

    property real font_size: height*.04

    property real side_size: height*.1
    property real margins: side_size*.1


    //    Button {
    //        id: b_about
    //        width: navigation_block.width
    //        height: side_size
    //        //        anchors.top: b_up.bottom
    //        //        anchors.left: b_up.right
    //        anchors.horizontalCenter: parent.horizontalCenter
    //        anchors.margins: margins
    //        text: "about"
    //    }

    Button {
        id: b_info
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: b_rec_state.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "info"
        onClicked: messenger.keyPressed(KeyCode.Info)
    }

    Button {
        id: b_rec_state
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: b_menu.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "rec state"
        onClicked: messenger.keyPressed(KeyCode.RecState)
    }

    Button {
        id: b_menu
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: navigation_block.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "menu"
        onClicked: messenger.keyPressed(KeyCode.Menu)
    }

    Button {
        id: b_back
        width: navigation_block.width - margins*4
        height: side_size
        anchors.top: navigation_block.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "back"
        onClicked: messenger.keyPressed(KeyCode.Back)
    }

    Rectangle {
        id: navigation_block
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height*.42
        width: side_size*3 + margins*4
        height: side_size*3 + margins*4
        // color: "green"
        color: "transparent"

        Button {
            id: b_left
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.right: b_up.left
            anchors.margins: margins
            text: "<"
            onClicked: messenger.keyPressed(KeyCode.Left)
        }

        Button {
            id: b_right
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.left: b_up.right
            anchors.margins: margins
            text: ">"
            onClicked: messenger.keyPressed(KeyCode.Right)
        }

        Button {
            id: b_up
            width: side_size
            height: side_size
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            text: "^"
            onClicked: messenger.keyPressed(KeyCode.Up)
        }

        Button {
            id: b_down
            width: side_size
            height: side_size
            anchors.top: b_enter.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            text: "v"
            onClicked: messenger.keyPressed(KeyCode.Down)
        }

        Button {
            id: b_enter
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            text: "enter"
            onClicked: messenger.keyPressed(KeyCode.Enter)
        }
    }
}
