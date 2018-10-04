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


Rectangle {
    id: root

    color: "transparent"

    property int max_height: height - title_left.height

    Rectangle {
        id: line_left

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_left.top
        anchors.horizontalCenter: title_left.horizontalCenter
    }

    Rectangle {
        id: line_right

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_right.top
        anchors.horizontalCenter: title_right.horizontalCenter
    }

    Rectangle {
        id: line_center

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_center.top
        anchors.horizontalCenter: title_center.horizontalCenter
    }

    Rectangle {
        id: line_lfe

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_lfe.top
        anchors.horizontalCenter: title_lfe.horizontalCenter
    }

    Rectangle {
        id: line_rear_left

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_back_left.top
        anchors.horizontalCenter: title_back_left.horizontalCenter
    }

    Rectangle {
        id: line_rear_right

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_back_right.top
        anchors.horizontalCenter: title_back_right.horizontalCenter
    }

    Rectangle {
        id: line_side_left

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_side_left.top
        anchors.horizontalCenter: title_side_left.horizontalCenter
    }

    Rectangle {
        id: line_side_right

        color: "blue"

        width: title_left.width*.9
        height: 0

        anchors.bottom: title_side_right.top
        anchors.horizontalCenter: title_side_right.horizontalCenter
    }


    Text {
        id: title_left

        anchors.bottom: parent.bottom
        anchors.left: parent.left

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter

        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "L"
    }

    Text {
        id: title_right

        anchors.bottom: parent.bottom
        anchors.left: title_left.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter

        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "R"
    }

    Text {
        id: title_center

        anchors.bottom: parent.bottom
        anchors.left: title_right.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter

        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "C"
    }

    Text {
        id: title_lfe

        anchors.bottom: parent.bottom
        anchors.left: title_center.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter


        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "LFE"
    }

    Text {
        id: title_back_left

        anchors.bottom: parent.bottom
        anchors.left: title_lfe.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter


        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "RL"
    }

    Text {
        id: title_back_right

        anchors.bottom: parent.bottom
        anchors.left: title_back_left.right


        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter

        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "RR"
    }

    Text {
        id: title_side_left

        anchors.bottom: parent.bottom
        anchors.left: title_back_right.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter


        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "SL"
    }

    Text {
        id: title_side_right

        anchors.bottom: parent.bottom
        anchors.left: title_side_left.right

        width: parent.width/8

        horizontalAlignment: Text.AlignHCenter


        color: "white"

        font.pixelSize: parent.height*.1
        font.bold : true

        style: Text.Outline

        text: "SR"
    }

    function setLevel(l, r, c, lfe, rl, rr, sl, sr) {
        if(!visible)
            return

        line_left.height=l*root.max_height
        line_right.height=r*root.max_height
        line_center.height=c*root.max_height
        line_lfe.height=lfe*root.max_height
        line_rear_left.height=rl*root.max_height
        line_rear_right.height=rr*root.max_height
        line_side_left.height=sl*root.max_height
        line_side_right.height=sr*root.max_height
    }
}
