import QtQuick 2.7

Rectangle {
    id: menu_header

    visible: opacity!=0.
    enabled: visible
    opacity: 0.

    property bool state_visible: false

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
}
