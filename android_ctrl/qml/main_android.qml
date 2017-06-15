import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0
import QtGraphicalEffects 1.0

import FuckTheSystem 0.0

ApplicationWindow {
    id: root

    visible: true

    width: 360
    height: 640

    title: "Capturer Control"

    property real font_size: height*.028

    LinearGradient {
        anchors.fill: parent

        start: Qt.point(width, 0)
        end: Qt.point(0, height)

        gradient: Gradient { // UbuntuTouch-like background
            GradientStop { position: 0.2; color: "#c05c7c"; }
            GradientStop { position: 0.8; color: "#d56a59"; }
        }
    }

    SetupForm {
        id: setup_form
        anchors.fill: parent
    }

    SwipeView {
        id: swipe_view
        currentIndex: 1
        anchors.fill: parent

        Item {
            AdditionalControl {
                id: zeroPage
                anchors.fill: parent
            }
        }

        Item {
            ButtonRec {
                id: button_rec
                width: parent.width<parent.height ? parent.width*.5 : parent.height*.5
                height: width
                y: root.height*.43
                anchors.horizontalCenter: parent.horizontalCenter
                onClickEvent: messenger.keyPressed(KeyCode.Rec)
            }
        }

        Item {
            BaseControl {
                id: secondPage
                anchors.fill: parent
            }
        }

        Item {
            Player {
                id: player
                anchors.fill: parent
            }
        }
    }

    PageIndicator {
        id: swipe_pos_indicator

        count: swipe_view.count
        currentIndex: swipe_view.currentIndex

        anchors.bottom: swipe_view.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Connections {
        target: setup_form

        onBack: {
            setup_form.visible=false
            swipe_view.visible=true
            swipe_pos_indicator.visible=true
        }
    }

    Connections {
        target: messenger

        onUpdateRecStats: {
            button_rec.setText("duration: " + duration + "\n" + "size: " + size + "\n" + "bitrate: " + bitrate)
        }

        onRecStateChanged: {
            if(state)
                button_rec.recStarted()

            else
                button_rec.recStopped()
        }
    }

    MenuBar {
        Menu {
            title: "setup"

            MenuItem {
                id: m_setup
                text: "setup"

                onTriggered: {
                    hideAll()
                    setup_form.visible=true
                    swipe_view.visible=false
                    swipe_pos_indicator.visible=false
                }
            }
        }
    }

    function hideAll() {
        setup_form.visible=false
    }
}
