import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    id: root

    visible: false

    property real font_size: height*.034

    signal back

    GridLayout {
        id: grid_layout
        rows: 2
        flow: GridLayout.TopToBottom

        width: parent.width*.8

        anchors.horizontalCenter: parent.horizontalCenter

        y: parent.height*.24

        Label {
            font.pixelSize: font_size
            text: "host:"
        }

        Label {
            font.pixelSize: font_size
            text: "port:"
        }

        /*
        Label {
            font.pixelSize: font_size
            text: "routing key:"
        }
        */

        TextField {
            id: tf_host
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectAddrHost();
        }

        TextField {
            id: tf_port
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectAddrPort();
        }

        /*
        TextField {
            id: tf_routing_key
            font.pixelSize: font_size
            Layout.fillWidth: true
            text: messenger.connectRoutingKey();
        }
        */
    }

    Button2 {
        id: b_apply
        width: parent.width*.5
        height: parent.height*.1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: root.height*.1
        font_pixel_size: font_size

        text: "apply"

        onClicked: {
            messenger.setConnectParams(tf_host.text, tf_port.text, tf_routing_key.text)
            root.back()
        }
    }
}
