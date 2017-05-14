import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3

Rectangle {
    id: no_signal
    anchors.fill: parent

    property real row_1: .9
    property real row_2: .1

    visible: false

    GridLayout {
        columns: 1
        rows: 2

        anchors.fill: parent

        rowSpacing: 0
        columnSpacing: 0

        GridLayout {
            columns: 7

            rowSpacing: 0
            columnSpacing: 0

            Rectangle {
                color: "#ffffffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ffffff00"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ff00ffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ff00ff00"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ffff00ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ffff0000"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }

            Rectangle {
                color: "#ff0000ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_1
            }
        }

        GridLayout {
            columns: 7

            rowSpacing: 0
            columnSpacing: 0

            Rectangle {
                color: "#ff0000ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ff000000"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ffff00ff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ff555555"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ff00ffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ffaaaaaa"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }

            Rectangle {
                color: "#ffffffff"
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: no_signal.height*no_signal.row_2
            }
        }
    }

    Rectangle{
        color: "#ff222222"
        width: parent.width*.57
        height: parent.height*.25
        anchors.centerIn: parent

        Text {
            color: "white"
            anchors.centerIn: parent
            font.pixelSize: (parent.width + parent.height)/2*.2
            font.bold: true
            text: "no signal"
        }
    }

    Connections {
        target: messenger
        onSignalLost: no_signal.visible=value
    }
}
