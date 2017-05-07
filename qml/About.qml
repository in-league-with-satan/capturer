import QtQuick 2.7

Rectangle {
    id: about

    color: "#bb000000"

    property string version_software: messenger.versionThis()
    property string version_avutil: messenger.versionLibAVUtil()
    property string version_avcodec: messenger.versionlibAVCodec()
    property string version_avformat: messenger.versionlibAVFormat()
    property string version_avfilter: messenger.versionlibAVFilter()
    property string version_swscale: messenger.versionlibSWScale()
    property string version_swresample: messenger.versionlibSWResample()

    property bool state_visible: false


    Text {
        font.pixelSize: (parent.width + parent.height)/2*.04
        anchors.centerIn: parent
        color: "white"
        text: "versions\n\n" +
              "software: " + version_software + "\n" +
              "avutil: " + version_avutil + "\n" +
              "avcodec: " + version_avcodec + "\n" +
              "avformat: " + version_avformat + "\n" +
              "avfilter: " + version_avfilter + "\n" +
              "swscale: " + version_swscale + "\n" +
              "swresample: " + version_swresample
    }

    states: [
        State {
            when: state_visible

            PropertyChanges {
                target: about
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: about
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
