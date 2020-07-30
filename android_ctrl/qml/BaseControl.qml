/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

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
        id: b_info
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: b_rec_state.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "info"
        onClicked: messenger.keyPressed(KeyCode.Info)
    }

    Button2 {
        id: b_rec_state
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: b_file_browser.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "rec state"
        onClicked: messenger.keyPressed(KeyCode.RecState)
    }

    Button2 {
        id: b_file_browser
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: b_menu.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "file browser"
        onClicked: messenger.keyPressed(KeyCode.FileBrowser)
    }

    Button2 {
        id: b_menu
        width: navigation_block.width - margins*4
        height: side_size
        anchors.bottom: navigation_block.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "menu"
        onClicked: messenger.keyPressed(KeyCode.Menu)
    }

    Button2 {
        id: b_back
        width: navigation_block.width - margins*4
        height: side_size
        anchors.top: navigation_block.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: margins
        font_pixel_size: font_size
        text: "back"
        onClicked: messenger.keyPressed(KeyCode.Back)
    }

    Rectangle {
        id: navigation_block
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height*.47
        width: side_size*3 + margins*4
        height: side_size*3 + margins*4
        // color: "green"
        color: "transparent"

        Button2 {
            id: b_left
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.right: b_up.left
            anchors.margins: margins
            font_pixel_size: font_size
            text: "<"
            onClicked: messenger.keyPressed(KeyCode.Left)
        }

        Button2 {
            id: b_right
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.left: b_up.right
            anchors.margins: margins
            font_pixel_size: font_size
            text: ">"
            onClicked: messenger.keyPressed(KeyCode.Right)
        }

        Button2 {
            id: b_up
            width: side_size
            height: side_size
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            font_pixel_size: font_size
            text: "^"
            onClicked: messenger.keyPressed(KeyCode.Up)
        }

        Button2 {
            id: b_down
            width: side_size
            height: side_size
            anchors.top: b_enter.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            font_pixel_size: font_size
            text: "v"
            onClicked: messenger.keyPressed(KeyCode.Down)
        }

        Button2 {
            id: b_enter
            width: side_size
            height: side_size
            anchors.top: b_up.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: margins
            font_pixel_size: font_size
            text: "enter"
            onClicked: messenger.keyPressed(KeyCode.Enter)
        }
    }
}
