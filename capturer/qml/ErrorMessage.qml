/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
import QtQuick.Controls 2.1

import FuckTheSystem 0.0


ShowHideRect {
    id: root
    color: "#bb000000"
    clip: true


    Text {
        id: message
        color: "white"
        anchors.fill: parent
        font.pixelSize: (root.width + root.height)/2*.04
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }

    Connections {
        target: messenger

        function onShowMenu() { root.state_visible=false }
        function onShowFileBrowser() { root.state_visible=false }
        function onShowHideAbout() { root.state_visible=false }
        function onShowHideInfo() { root.state_visible=false }
        function onRecStarted() { root.state_visible=false }
        function onBack() { root.state_visible=false }

        function onErrorString(error_string) {
            message.text=error_string
            root.state_visible=true
        }
    }
}
