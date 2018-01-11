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
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

Button {
    id: root
    clip: true

    property real font_pixel_size: height*.6

    style: ButtonStyle {
        label: Text {
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: font_pixel_size
            text: control.text
        }
    }

    Timer {
        id: press_timer
        interval: 150
        repeat: true
        running: false
        onTriggered: root.clicked()
    }

    onPressedChanged: press_timer.running=pressed
}
