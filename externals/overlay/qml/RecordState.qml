import QtQuick 2.7
import QtQuick.Window 2.2


Rectangle {
    id: rec_state_bar

    visible: true
    width: parent.width - parent.height*.08*.5
    height: parent.height*.02

    property bool state_visible: false

    color: "transparent"


    Text {
        id: text_output

        anchors.left: circle.right
        anchors.leftMargin: parent.height*.8
        y: parent.height*.02
        color: "white"
        font.pixelSize: parent.height*.8
        font.bold : true
        style: Text.Outline

        text: "00:10:50.234    120 mbit/s    frames dropped: 0"
    }

    Rectangle {
        id: circle

        width: Screen.desktopAvailableHeight*.02
        height: Screen.desktopAvailableHeight*.02
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
                messenger.hello();

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
                opacity: 1.0
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: rec_state_bar
                opacity: 0.0
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
           text_output.text=duration + "    " + size + " bytes" + "    " + bitrate + " mbit/s";
       }
    }
}
