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

Item {
    id: root

    visible: false

    property real font_size: height*.034

    signal back

    GridLayout {
        id: grid_layout
        rows: 2
        flow: GridLayout.TopToBottom

        width: parent.width*.8

        anchors.horizontalCenter: parent.horizontalCenter

        y: parent.height*.24

        Label {
            font.pixelSize: font_size
            text: "host:"
        }

        Label {
            font.pixelSize: font_size
            text: "port:"
        }

        /*
        Label {
            font.pixelSize: font_size
            text: "routing key:"
        }
        */

        TextField {
            id: tf_host
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectAddrHost();
        }

        TextField {
            id: tf_port
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectAddrPort();
        }

        /*
        TextField {
            id: tf_routing_key
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectRoutingKey();
        }
        */
    }

    Button2 {
        id: b_apply
        width: parent.width*.5
        height: parent.height*.1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: root.height*.1
        font_pixel_size: font_size

        text: "apply"

        onClicked: {
            messenger.setConnectParams(tf_host.text, tf_port.text, "")
            root.back()
        }
    }
}
