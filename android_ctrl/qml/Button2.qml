import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

Button {
    id: root
    clip: true

    property real font_pixel_size: height*.6

    style: ButtonStyle {
        label: Text {
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: font_pixel_size
            text: control.text
        }
    }

    Timer {
        id: press_timer
        interval: 150
        repeat: true
        running: false
        onTriggered: root.clicked()
    }

    onPressedChanged: press_timer.running=pressed
}
