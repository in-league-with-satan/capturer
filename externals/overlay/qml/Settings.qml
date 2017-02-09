import QtQuick 2.7
import QtQuick.Window 2.2
//import QtQuick.Controls 1.5
import QtQuick.Controls 2.1

Rectangle {
    id: settings

    color: "#bb000000"

    property bool state_visible: false

    Grid {
        columns: 2
        spacing: Screen.height*.04

        anchors.centerIn: parent

        horizontalItemAlignment: Grid.AlignRight

        //

        Text {
            id: l_video_encoder
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Video encoder:"
        }

        ComboBox {
            id: cb_video_encoder
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            model: messenger.modelVideoEncoder

            onCurrentIndexChanged: messenger.onVideoCodecIndexChanged(currentIndex)

            Connections {
                target: messenger
                onVideoEncoderIndexSet: cb_video_encoder.currentIndex=index
            }
        }

        //

        Text {
            id: l_pixel_format
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Pixel format:"
        }


        ComboBox {
            id: cb_pixel_format
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            model: messenger.modelPixelFormat

            onCurrentIndexChanged: messenger.onPixelFormatIndexChanged(currentIndex)

            Connections {
                target: messenger
                onPixelFormatIndexSet: cb_pixel_format.currentIndex=index
            }
        }

        //

        Text {
            id: l_crf
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Constant rate factor / quality:"
        }

        ComboBox {
            id: cb_crf
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            model: [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 ]

            onCurrentIndexChanged: messenger.onCrfChanged(currentIndex)

            Connections {
                target: messenger
                onCrfSet: cb_crf.currentIndex=value
            }
        }

        //

        Text {
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Half-fps:"
        }

        CheckBox {
            id: cb_half_fps
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            scale: parent.height*.003
            onCheckStateChanged: messenger.onHalfFpsChanged(checked)

            Connections {
                target: messenger
                onHalfFpsSet: cb_half_fps.checked=value
            }
        }

        //

        Text {
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Stop rec on frames drop:"
        }

        CheckBox {
            id: cb_stop_on_drop
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            scale: parent.height*.003
            onCheckStateChanged: messenger.onStopOnDropChanged(checked)

            Connections {
                target: messenger
                onStopOnDropSet: cb_stop_on_drop.checked=value
            }
        }
    }

    //

    states: [
        State {
            when: state_visible;

            PropertyChanges {
                target: settings
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: settings
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

        onShowMenu: settings.state_visible=true
        onBack: settings.state_visible=false
    }
}

