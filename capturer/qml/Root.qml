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
import QtQuick.Window 2.2
import QtMultimedia 5.8


Rectangle {
    id: root
    visible: true
    color: "black"

    VideoOutput {
        id: output_primary
        anchors.fill: parent
        source: messenger.videoSourcePrimary()
    }

    NoSignal {}

    ShaderEffect {
        id: shader_effect
        anchors.fill: output_primary
        visible: false

        property real m_brightness: 1.
        property real m_saturation: 2.

        property variant source: ShaderEffectSource {
            sourceItem: output_primary
        }

        fragmentShader: "
            // precision mediump float;
            precision highp float;

            uniform sampler2D source;
            varying vec2 qt_TexCoord0;

            float gamma=.65;
            float white=.65;

            uniform float m_brightness;
            uniform float m_saturation;

            vec3 saturation(vec3 rgb, float adjustment)
            {
                const vec3 W=vec3(.2125, .7154, .0721);
                vec3 intensity=vec3(dot(rgb, W));
                return mix(intensity, rgb, adjustment);
            }

            vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
            {
                float luma=dot(color, vec3(.2126, .7152, .0722));
                float tone_mapped_luma=luma*(1. + luma/(white*white))/(1. + luma);
                color*=tone_mapped_luma/luma;

                gamma=m_brightness;

                color=pow(color, vec3(1./gamma));

                return saturation(color, m_saturation);
            }

            void main()
            {
                vec2 uv=qt_TexCoord0.xy;
                vec4 orig=texture2D(source, uv);
                gl_FragColor=vec4(whitePreservingLumaBasedReinhardToneMapping(orig.rgb), 1.);
            }"
    }

    VideoOutput {
        id: output_secondary
        x: parent.width*.06
        y: parent.height - height - parent.height*.1
        width: parent.width*.3
        height: parent.height*.3
        source: messenger.videoSourceSecondary()
        property int position: 0
    }

    ErrorMessage {
        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent
    }

    About {
        id: about
        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent
    }

    MenuHeader {
        id: menu_header

        x: 0
        y: 0

        width: parent.width
        height: parent.height*.1
    }

    Settings {
        id: settings

        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent
    }

    FileBrowser {
        id: file_browser

        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent
    }

    RecordState {
        id: rec_state_bar

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height - height*1.5
    }

    PlayerState {
        width: parent.width*.8
        height: parent.height*.075
        y: parent.height*.8
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Connections {
        target: messenger

        onShowMenu: {
            if(rec_state_bar.state_visible)
                return

            settings.state_visible=true
            menu_header.state_visible=true

            about.state_visible=false
            file_browser.state_visible=false
        }

        onShowFileBrowser: {
            if(file_browser.state_visible)
                return

            file_browser.state_visible=true

            about.state_visible=false
            settings.state_visible=false
            menu_header.state_visible=false
        }

        onShowHideAbout: {
            about.state_visible=!about.state_visible

            if(about.state_visible) {
                settings.state_visible=false
                menu_header.state_visible=false
                file_browser.state_visible=false
            }
        }

        onShowHideInfo: menu_header.state_visible=!menu_header.state_visible

        onRecStarted: {
            settings.state_visible=false
            menu_header.state_visible=false
            file_browser.state_visible=false
            about.state_visible=false
        }

        onBack: {
            about.state_visible=false
            menu_header.state_visible=false
            settings.state_visible=false
            file_browser.state_visible=false
        }

        onErrorString: {
            about.state_visible=false
            menu_header.state_visible=false
            settings.state_visible=false
            file_browser.state_visible=false
        }

        onPreviewSecondary: {
            output_secondary.visible=visible
        }

        onPreviewSecondaryChangePosition: {
            output_secondary.position++;

            if(output_secondary.position>1)
                output_secondary.position=0;

            if(output_secondary.position==0) {
                output_secondary.x=Qt.binding(function() { return root.width*.06 })
                output_secondary.y=Qt.binding(function() { return root.height - output_secondary.height - root.height*.1 })
                output_secondary.width=Qt.binding(function() { return root.width*.3 })
                output_secondary.height=Qt.binding(function() { return root.height*.3 })
            }

            if(output_secondary.position==1) {
                output_secondary.x=0
                output_secondary.y=0
                output_secondary.width=Qt.binding(function() { return root.width})
                output_secondary.height=Qt.binding(function() { return root.height })
            }
        }

        onSetHdrToSdrEnabled: {
            shader_effect.visible=value;
        }

        onSetHdrBrightness: {
            shader_effect.m_brightness=value;
        }

        onSetHdrSaturation: {
            shader_effect.m_saturation=value;
        }
    }
}
