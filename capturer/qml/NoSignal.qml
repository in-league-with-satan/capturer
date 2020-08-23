/******************************************************************************

Copyright © 2018, 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3


Rectangle {
    id: root

    anchors.fill: parent

    property real row_1: .9
    property real row_2: .1

    visible: false

    GridLayout {
        columns: 1
        rows: 2

        anchors.fill: parent

        rowSpacing: 0
        columnSpacing: 0

        GridLayout {
            columns: 7

            rowSpacing: 0
            columnSpacing: 0

            Rectangle {
                color: "#ffffffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ffffff00"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ff00ffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ff00ff00"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ffff00ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ffff0000"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }

            Rectangle {
                color: "#ff0000ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_1
            }
        }

        GridLayout {
            columns: 7

            rowSpacing: 0
            columnSpacing: 0

            Rectangle {
                color: "#ff0000ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ff000000"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ffff00ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ff555555"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ff00ffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ffaaaaaa"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }

            Rectangle {
                color: "#ffffffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: root.height*root.row_2
            }
        }
    }

    Rectangle{
        color: "#ff222222"
        width: parent.width*.57
        height: parent.height*.25
        anchors.centerIn: parent

        Text {
            color: "white"
            anchors.centerIn: parent
            font.pixelSize: (parent.width + parent.height)/2*.2
            font.bold: true
            text: "no signal"
        }
    }

    Connections {
        target: messenger

        function onSignalLost(value) {
            root.visible=value
        }
    }
}
