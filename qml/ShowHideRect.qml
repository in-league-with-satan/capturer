import QtQuick 2.7

Rectangle {
    id: root

    visible: opacity!=0.
    enabled: visible
    opacity: 0.

    property bool state_visible: false

    states: [
        State {
            when: state_visible;

            PropertyChanges {
                target: root
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: root
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
