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
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    color: "transparent"

    property bool rec_running: false
    property alias text: text_stats.text

    signal clickEvent

    function recStarted() {
        anm_show_glow.start()
        anm_color_lighter.start()
        // scale_animation_pause.start()
        anm_text_show.start()
        fake_rect.border_size=Qt.binding(function() { return root.width*.08 })
        rec_running=true
    }

    function recStopped() {
        anm_hide_glow.start()
        anm_color_darker.start()
        anm_text_hide.start()
        rec_running=false
    }

    function setText(text) {
        text_stats.text=text

        if(!rec_running)
            recStarted()
    }

    Text {
        id: text_stats
        color: "white"
        width: root.parent.width
        anchors.horizontalCenter: root.horizontalCenter
        anchors.bottom: root.top
        anchors.margins: width*.07
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: font_size
        font.bold: true
        style: Text.Outline
        opacity: 0

        PropertyAnimation {
            id: anm_text_show
            target: text_stats
            property: "opacity"
            from: text_stats.opacity
            to: 1
        }

        PropertyAnimation {
            id: anm_text_hide
            target: text_stats
            property: "opacity"
            from: text_stats.opacity
            to: 0
        }
    }

    ScaleAnimator {
        id: scale_animation_front
        target: fake_rect
        from: 1
        to: 1.1
        duration: 100

        onStopped: {
            if(rec_running)
                scale_animation_back.start()

            else
                scale_animation_back.start()
        }
    }

    ScaleAnimator {
        id: scale_animation_back
        target: fake_rect
        from: 1.1
        to: 1
        duration: 100

        onStopped: {
            if(rec_running)
                scale_animation_pause.start()
        }
    }

    ScaleAnimator {
        id: scale_animation_pause
        target: fake_rect
        from: 1
        to: 1
        duration: 1000

        onStopped: {
            if(rec_running)
                scale_animation_front.start()
        }
    }

    Item {
        id: fake_rect
        anchors.fill: parent

        property alias color: fake_background.color
        property int border_size : root.width*.08

        RectangularGlow {
            id: glow_effect
            anchors.fill: parent

            glowRadius: 10
            spread: 0.2
            opacity: 0
            color: "#ffbbaa"
            cornerRadius: Math.min (width, height)*.5

            PropertyAnimation {
                id: anm_show_glow
                target: glow_effect
                property: "opacity"
                from: glow_effect.opacity
                to: 1
            }

            PropertyAnimation {
                id: anm_hide_glow
                target: glow_effect
                property: "opacity"
                from: glow_effect.opacity
                to: 0
            }
        }

        Rectangle {
            id: fake_border
            anchors.fill: parent
            radius: Math.min (width, height)*.5
            antialiasing: true

            gradient: Gradient {
                GradientStop { position: 0.; color: Qt.lighter (fake_rect.color, 1.42); }
                GradientStop { position: 1.; color: Qt.darker (fake_rect.color, 1.42); }
            }

            Rectangle {
                id: fake_background
                color: "#aa0000"
                radius: Math.min (width, height)*.5
                antialiasing: true
                anchors {
                    fill: parent
                    margins: fake_rect.border_size
                }
            }

            PropertyAnimation {
                id: anm_color_lighter
                target: fake_background
                property: "color"
                from: fake_background.color
                to: "#ff0000"
            }

            PropertyAnimation {
                id: anm_color_darker
                target: fake_background
                property: "color"
                from: fake_background.color
                to: "#aa0000"
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        property int prev_pos_x: 0

        onPressed: {
            // if(rec_running)
            //     return

            prev_pos_x=mouse.x

            fake_rect.border_size=Qt.binding(function() { return root.width*.02 })
        }

        onReleased: {
            // if(rec_running)
            //     return

            fake_rect.border_size=Qt.binding(function() { return root.width*.08 })
        }

        onPositionChanged: {
            if(Math.abs(prev_pos_x - mouse.x)>10)
                onReleased(mouse)
        }

        onDoubleClicked: {
            clickEvent()
        }
    }
}
