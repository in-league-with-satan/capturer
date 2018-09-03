/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

import QtQuick 2.7
import QtQuick.Window 2.2


ShowHideRect {
    color: "#bb000000"

    LevelMonitor {
        width: parent.width*.2
        height: parent.height
    }

    Text {
        id: input_format

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height/2 - height

        color: "white"

        font.pixelSize: parent.height*.2
        font.bold: true

        style: Text.Outline

        text: "Input format:"
    }

    Text {
        id: free_space

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height/2 + height/2

        color: "white"

        font.pixelSize: parent.height*.2
        font.bold: true

        style: Text.Outline

        text: "free space:"
    }

    Text {
        id: t_time

        anchors.right: parent.right
        anchors.margins: parent.height*.2
        anchors.verticalCenter: parent.verticalCenter

        color: "white"

        font.pixelSize: parent.height*.4
        font.bold: true

        style: Text.Outline

        text: Qt.formatTime(new Date(), "hh:mm")

        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: t_time.text=Qt.formatTime(new Date(), "hh:mm")
        }
    }

    Connections {
        target: messenger

        onFormatChanged: input_format.text="input format: " + format
        onFreeSpaceStr: free_space.text="free space: " + size
    }
}
