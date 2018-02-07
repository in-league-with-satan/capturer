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
import QtQuick.Controls 2.3

ShowHideRect {
    id: root
    color: "#bb000000"

    ListView {
        anchors.fill: parent
        clip: true

        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            anchors.right: parent.right
            width: { return parent.width*.01<12 ? 12 : parent.width*.01 }
            stepSize: .024
            policy: ScrollBar.AlwaysOn
        }

        model: ListModel {
            Component.onCompleted: {
                var addr=messenger.networkAddresses();

                append({ first: "", second: "", title: "network addresses" })

                for(var i=0; i<addr.length; i++)
                    append({ first: addr[i], second: "", title: "" })

                //

                append({ first: "", second: "", title: "" })

                //

                append({ first: "", second: "", title: "versions" })
                append({ first: "software", second: messenger.versionThis(), title: "" })
                append({ first: "avutil", second: messenger.versionLibAVUtil(), title: "" })
                append({ first: "avcodec", second: messenger.versionlibAVCodec(), title: "" })
                append({ first: "avformat", second: messenger.versionlibAVFormat(), title: "" })
                append({ first: "avfilter", second: messenger.versionlibAVFilter(), title: "" })
                append({ first: "swscale", second: messenger.versionlibSWScale(), title: "" })
                append({ first: "swresample", second: messenger.versionlibSWResample(), title: "" })
            }
        }

        delegate: Rectangle {
            width: root.width
            height: root.height*.1

            property int font_size: height*.678

            color: "transparent"

            Text {
                color: "white"
                anchors.fill: parent
                font.pixelSize: font_size
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: title
            }

            Text {
                color: "white"
                x: parent.width*.2
                width: parent.width*.2
                height: parent.height
                font.pixelSize: font_size
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
                // clip: true
                text: first
            }

            Text {
                color: "white"
                x: parent.width*.6
                width: parent.width*.2
                height: parent.height
                font.pixelSize: font_size
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                // clip: true
                text: second
            }
        }
    }

    Connections {
        target: messenger

        onKeyPressed: {
            if(!root.state_visible)
                return;

            switch(key) {
            case Qt.Key_Up:
                scrollbar.decrease()
                break

            case Qt.Key_Down:
                scrollbar.increase()
                break

            case Qt.Key_Left:
                scrollbar.position=0
                break

            case Qt.Key_Right:
                scrollbar.position=scrollbar.size
                scrollbar.increase()
                break

            default:
                break
            }
        }
    }
}
