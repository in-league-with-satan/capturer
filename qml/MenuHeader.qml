import QtQuick 2.7
import QtQuick.Window 2.2


Rectangle {
    id: menu_header

    visible: true

    property bool state_visible: false

    color: "#bb000000"


    LevelMonitor {
        width: parent.width*.2
        height: parent.height
    }

    Text {
        id: input_format

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height/2 - height

        color: "white"

        font.pixelSize: parent.height*.2
        font.bold: true

        style: Text.Outline

        text: "Input format:"
    }

    Text {
        id: free_space

        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height/2 + height/2

        color: "white"

        font.pixelSize: parent.height*.2
        font.bold: true

        style: Text.Outline

        text: "free space:"
    }

    Text {
        id: t_time

        anchors.right: parent.right
        anchors.margins: parent.height*.2
        anchors.verticalCenter: parent.verticalCenter

        color: "white"

        font.pixelSize: parent.height*.4
        font.bold: true

        style: Text.Outline

        text: Qt.formatTime(new Date(), "hh:mm")

        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: t_time.text=Qt.formatTime(new Date(), "hh:mm")
        }
    }


    states: [
        State {
            when: state_visible;

            PropertyChanges {
                target: menu_header
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: menu_header
                opacity: 0.
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                property: "opacity"
                duration: 500
            }
        }
    ]

    Connections {
        target: messenger

        onFormatChanged: input_format.text="input format: " + height + (progressive_frame ? "p" : "i") + "@" + (frame_scale/frame_duration).toFixed(2) + " " + pixel_format
        onFreeSpace: free_space.text="free space: " + size
    }
}
