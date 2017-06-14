import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import FuckTheSystem 0.0

Item {
    id: root

    property real font_size: height*.032
    property real side_size: height*.1
    property real margins: side_size*.1


    Button2 {
        id: b_about
        width: side_size*3 + margins*4
        height: side_size
        y: parent.height*.34
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "about"
        onClicked: messenger.keyPressed(KeyCode.About)
    }

    Button2 {
        id: b_preview
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_about.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "preview"
        onClicked: messenger.keyPressed(KeyCode.Preview)
    }

    Button2 {
        id: b_fullscreen
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_preview.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "fullscreen"
        onClicked: messenger.keyPressed(KeyCode.FullScreen)
    }

    Button2 {
        id: b_exit
        width: side_size*3 + margins*4
        height: side_size
        anchors.top: b_fullscreen.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "exit"
        onClicked: messenger.keyPressed(KeyCode.Exit)
    }
}