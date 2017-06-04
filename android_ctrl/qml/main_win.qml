import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3


import FuckTheSystem 0.0

ApplicationWindow {
    id: root

    visible: true

    width: 360
    height: 640

    title: "Capturer Control"

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
            Rectangle {
                id: button_rec
                width: parent.width<parent.height ? parent.width*.5 : parent.height*.5
                height: width

                anchors.centerIn: parent

                color: "red"

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton;
                    onDoubleClicked: messenger.keyPressed(KeyCode.Rec)
                }
            }
        }

        Item {
            BaseControl {

                id: secondPage
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

    menuBar: MenuBar {
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
