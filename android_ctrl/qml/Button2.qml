import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

Button {
    clip: true

    property real pixelSize: height*.6

    style: ButtonStyle {
        label: Text {
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: pixelSize
            text: control.text
        }
    }
}
