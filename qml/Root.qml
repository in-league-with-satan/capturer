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
import QtMultimedia 5.8


Rectangle {
    id: root
    visible: true
    color: "black"

    VideoOutput {
        anchors.fill: parent;
        source: messenger.videoSourceMain()
    }

    NoSignal {}

    VideoOutput {
        id: cam_output
        x: parent.width*.06
        y: parent.height - height - parent.height*.1
        width: parent.width*.3
        height: parent.height*.3
        source: messenger.videoSourceCam()
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

        onCamPreview: {
            cam_output.visible=visible
        }

        onCamPreviewChangePosition: {
            cam_output.position++;

            if(cam_output.position>1)
                cam_output.position=0;

            if(cam_output.position==0) {
                cam_output.x=Qt.binding(function() { return root.width*.06 })
                cam_output.y=Qt.binding(function() { return root.height - cam_output.height - root.height*.1 })
                cam_output.width=Qt.binding(function() { return root.width*.3 })
                cam_output.height=Qt.binding(function() { return root.height*.3 })
            }

            if(cam_output.position==1) {
                cam_output.x=0
                cam_output.y=0
                cam_output.width=Qt.binding(function() { return root.width})
                cam_output.height=Qt.binding(function() { return root.height })
            }
        }
    }
}
