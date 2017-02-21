import QtQuick 2.7
import QtQuick.Window 2.2


Rectangle {
    id: rec_state_bar

    visible: true
    width: parent.width*.96
    height: parent.height*.04

    property bool state_visible: false

    // color: "blue"
    color: "transparent"

    Text {
        id: text_output

        anchors.left: circle.right
        anchors.leftMargin: parent.height*.5
        anchors.verticalCenter: parent.verticalCenter

        color: "white"

        font.pixelSize: parent.height*.8
        font.bold: true

        style: Text.Outline

        text: ""
    }

    Rectangle {
        id: circle

        width: parent.height
        height: parent.height
        color: "red"

        radius: width*.5

        ScaleAnimator {
            id: scale_animation_front
            target: circle;
            from: .9;
            to: 1;
            duration: 100
            running: true

            onStopped: {
                scale_animation_back.start();
            }
        }

        ScaleAnimator {
            id: scale_animation_back
            target: circle;
            from: 1;
            to: .9;
            duration: 100

            onStopped: {
                scale_animation_pause.start();
            }
        }

        ScaleAnimator {
            id: scale_animation_pause
            target: circle;
            from: circle.scale;
            to: circle.scale;
            duration: 1000

            onStopped: {
                scale_animation_front.start();
            }
        }
    }

    states: [
        State {
            when: state_visible;

            PropertyChanges {
                target: rec_state_bar
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: rec_state_bar
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

        onUpdateRecStats: {
            text_output.text=duration + "    " + size + "    " + bitrate + "    " + buffer_state + "    " + dropped_frames_counter;
        }

        onRecStarted: rec_state_bar.state_visible=true
        onRecStopped: rec_state_bar.state_visible=false
    }
}
