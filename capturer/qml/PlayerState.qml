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


ShowHideRect {
    id: root

    color: "transparent"

    property int duration: 100000
    property int position: 9900
    property int seek_position: 0

    // Rectangle { anchors.fill: parent; color: "#aa00ff00" }

    Rectangle {
        id: progress_background
        anchors.fill: progress_contur
        color: "#88ffffff"
    }

    Rectangle {
        id: progress
        property real border_width: parent.height*.03
        x: border_width
        y: border_width
        width: (parent.width - border_width*2)*(root.position/root.duration)
        height: parent.height*.3 - border_width
        anchors.verticalCenter: parent.verticalCenter
        antialiasing: true
        color: "lightblue"
    }

    Rectangle {
        id: progress_contur
        width: parent.width
        height: parent.height*.3
        anchors.verticalCenter: parent.verticalCenter
        border.width: parent.height*.03
        border.color: "black"
        color: "transparent"
    }

    Text {
        id: t_pos
        x: parent.width*(root.position/root.duration) - paintedWidth*.5
        anchors.top: progress.bottom
        font.pixelSize: parent.height*.3
        verticalAlignment: Text.AlignTop
        style: Text.Outline
        color: "white"
        text: {
            var date=new Date(null)
            date.setMilliseconds(root.position)
            return date.toISOString().substr(11, 8)
        }
    }

    Text {
        id: t_dur
        x: root.width - paintedWidth*.5
        font.pixelSize: parent.height*.3
        anchors.bottom: progress_contur.top
        verticalAlignment: Text.AlignTop
        style: Text.Outline
        color: "white"
        text: {
            var date=new Date(null)
            date.setMilliseconds(root.duration)
            return date.toISOString().substr(11, 8)
        }
    }

    Connections {
        target: messenger

        function onShowHidePlayerState() {
            root.state_visible=!root.state_visible
        }

        function onShowPlayerState(visible) {
            root.state_visible=visible
        }

        function onPlayerDurationChanged(duration) {
            root.duration=duration
        }

        function onPlayerPositionChanged(position) {
            root.position=position
        }
    }
}
