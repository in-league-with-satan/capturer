import QtQuick 2.7
import QtQuick.Window 2.2


ShowHideRect {
    id: root

    width: parent.width*.96
    height: parent.height*.04

    property bool hide_text: false

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
            running: root.state_visible

            onStopped: {
                if(root.state_visible)
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
                if(root.state_visible)
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
                if(root.state_visible)
                    scale_animation_front.start();
            }
        }
    }

    Connections {
        target: messenger

        onUpdateRecStats: {
            if(!hide_text)
                text_output.text=duration + "    " + size + "    " + bitrate + "    " + buffer_state + "    " + dropped_frames_counter;
        }

        onShowHideDetailedRecState: {
            hide_text=!hide_text

            if(hide_text)
                text_output.text=""
        }

        onRecStarted: root.state_visible=true
        onRecStopped: root.state_visible=false
    }
}
