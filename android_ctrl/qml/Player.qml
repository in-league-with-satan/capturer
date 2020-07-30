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
