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
import QtQuick.Controls 1.4

ShowHideRect {
    id: root
    color: "#bb000000"

    property string version_software: messenger.versionThis()
    property string version_avutil: messenger.versionLibAVUtil()
    property string version_avcodec: messenger.versionlibAVCodec()
    property string version_avformat: messenger.versionlibAVFormat()
    property string version_avfilter: messenger.versionlibAVFilter()
    property string version_swscale: messenger.versionlibSWScale()
    property string version_swresample: messenger.versionlibSWResample()
    property string network_addresses: messenger.networkAddresses()
    property int scroll_step: (width + height)/2*.04

    ScrollView {
        id: scroll_view
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        // verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        Text {
            font.pixelSize: (root.width + root.height)/2*.04
            width: root.width*.9
            x: root.width*.1
            color: "white"
            text: "network addresses:\n" + network_addresses +
                  "\n\n" +
                  "versions\n\n" +
                  "software:\t\t" + version_software + "\n" +
                  "avutil:\t\t\t" + version_avutil + "\n" +
                  "avcodec:\t\t" + version_avcodec + "\n" +
                  "avformat:\t\t" + version_avformat + "\n" +
                  "avfilter:\t\t" + version_avfilter + "\n" +
                  "swscale:\t\t" + version_swscale + "\n" +
                  "swresample:\t" + version_swresample
        }
    }

    Connections {
        target: messenger

        onKeyPressed: {
            if(!root.state_visible)
                return;

            switch(key) {
            case Qt.Key_Up:
                if(scroll_view.flickableItem.contentY<(scroll_view.flickableItem.contentHeight>root.height ? scroll_view.flickableItem.contentHeight - root.height : 0))
                    scroll_view.flickableItem.contentY+=scroll_step

                break

            case Qt.Key_Down:
                if(scroll_view.flickableItem.contentY>0)
                    scroll_view.flickableItem.contentY-=scroll_step

                break

            case Qt.Key_Right:
                scroll_view.flickableItem.contentY=0

                break

            default:
                break
            }
        }
    }
}
