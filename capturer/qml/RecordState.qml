/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
    id: root

    width: parent.width*.96
    height: parent.height*.04

    // color: "blue"
    color: "transparent"

    ShowHideRect {
        id: rect_text_output
        state_visible: true
        anchors.left: circle.right
        anchors.leftMargin: parent.height*.5
        anchors.verticalCenter: parent.verticalCenter

        Text {
            id: text_output
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
            font.pixelSize: root.height*.7
            font.bold: true
            style: Text.Outline
            text: ""
        }
    }

    Rectangle {
        id: circle
        width: parent.height
        height: parent.height
        color: "red"
        radius: width*.5

        ScaleAnimator {
            id: scale_animation_front
            target: circle
            from: .9
            to: 1
            duration: 100
            // running: root.state_visible

            onStopped: {
                if(root.state_visible)
                    scale_animation_back.start()
            }
        }

        ScaleAnimator {
            id: scale_animation_back
            target: circle
            from: 1
            to: .9
            duration: 100

            onStopped: {
                if(root.state_visible)
                    scale_animation_pause.start()
            }
        }

        ScaleAnimator {
            id: scale_animation_pause
            target: circle
            from: circle.scale
            to: circle.scale
            duration: 1000

            onStopped: {
                if(root.state_visible)
                    scale_animation_front.start()
            }
        }
    }

    Connections {
        target: messenger

        onUpdateRecStats: {
            text_output.text=duration + "    " + size + "    " + bitrate + "    " + buffer_state + "    " + dropped_frames_counter
        }

        onShowHideDetailedRecState: {
            rect_text_output.state_visible=!rect_text_output.state_visible
        }

        onRecStarted: {
            root.state_visible=true
            rect_text_output.state_visible=true
        }

        onRecStopped: root.state_visible=false
    }
}
