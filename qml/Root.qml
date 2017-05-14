import QtQuick 2.7
import QtQuick.Window 2.2

import "qrc:/qml"


Rectangle {
    id: root

    visible: true

    color: "transparent"

    NoSignal {}

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

            file_browser.state_visible=false
        }

        onShowFileBrowser: {
            if(file_browser.state_visible)
                return

            file_browser.state_visible=true
            settings.state_visible=false
            menu_header.state_visible=false
        }

        onShowHideInfo: menu_header.state_visible=!menu_header.state_visible

        onRecStarted: settings.state_visible=false

        onBack: {
            menu_header.state_visible=false
            file_browser.state_visible=false
        }
    }
}

