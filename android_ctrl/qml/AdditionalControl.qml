import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import FuckTheSystem 0.0

Item {
    id: root

    property real font_size: height*.04

    property real side_size: height*.1
    property real margins: side_size*.1


    Button {
        id: b_about
        width: side_size*3 + margins*4
        height: side_size

        y: parent.height*.34
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "about"
        onClicked: messenger.keyPressed(KeyCode.About)
    }

    Button {
        id: b_preview
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_about.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "preview"
        onClicked: messenger.keyPressed(KeyCode.Preview)
    }

    Button {
        id: b_fullscreen
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_preview.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "fullscreen"
        onClicked: messenger.keyPressed(KeyCode.FullScreen)
    }

    Button {
        id: b_exit
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_fullscreen.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        text: "exit"
        onClicked: messenger.keyPressed(KeyCode.Exit)
    }
}
