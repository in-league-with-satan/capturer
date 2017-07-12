import QtQuick 2.7
import QtQuick.Controls 2.1

import FuckTheSystem 0.0


ShowHideRect {
    id: root
    color: "#bb000000"
    clip: true


    Text {
        id: message
        color: "white"
        anchors.fill: parent
        font.pixelSize: (root.width + root.height)/2*.04
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    Connections {
        target: messenger

        onShowMenu: root.state_visible=false
        onShowFileBrowser: root.state_visible=false
        onShowHideAbout: root.state_visible=false
        onShowHideInfo: root.state_visible=false
        onRecStarted: root.state_visible=false
        onBack: root.state_visible=false

        onErrorString: {
            message.text=error_string
            root.state_visible=true
        }
    }
}
