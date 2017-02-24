import QtQuick 2.7
import QtQuick.Window 2.2

import "qrc:/qml"


Rectangle {
    visible: true

    color: "transparent"

    width: Screen.width
    height: Screen.height


    MenuHeader {
        id: menu_header

        x: 0
        y: 0

        width: parent.width
        height: parent.height*.1

        opacity: 0
    }

    Settings {
        id: settings

        width: parent.width*.8
        height: parent.height*.8
        anchors.centerIn: parent

        opacity: 0
    }

    RecordState {
        id: rec_state_bar

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height - height*1.5

        opacity: 0
    }

    Connections {
        target: messenger

        onShowMenu: {
            if(rec_state_bar.state_visible)
                return

            settings.state_visible=true

            menu_header.state_visible=true
        }

        onShowHideInfo: menu_header.state_visible=!menu_header.state_visible

        onRecStarted: settings.state_visible=false

        onBack: menu_header.state_visible=false
    }
}

