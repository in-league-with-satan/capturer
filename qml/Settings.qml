import QtQuick 2.7
import QtQuick.Window 2.2
//import QtQuick.Controls 1.5
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3


ShowHideRect {
    id: settings

    color: "#bb000000"

    property int focus_index: field_id.video_encoder

    QtObject {
        id: field_id

        property int video_encoder: 0
        property int pixel_format: 1
        property int crf: 2
        property int half_fps: 3
        property int stop_on_drop: 4
        property int max: 4
    }

    //

    Rectangle {
        id: focus_indicator
        color: "#aa3355ff"

        property int y_video_encoder: grid.y + cb_video_encoder.y + cb_video_encoder.height/2 - focus_indicator.height/2
        property int y_pixel_format: grid.y + cb_pixel_format.y + cb_pixel_format.height/2 - focus_indicator.height/2
        property int y_crf: grid.y + cb_crf.y + cb_crf.height/2 - focus_indicator.height/2
        property int y_half_fps: grid.y + cb_half_fps.y + cb_half_fps.height/2 - focus_indicator.height/2
        property int y_stop_on_drop: grid.y + cb_stop_on_drop.y + cb_stop_on_drop.height/2 - focus_indicator.height/2

        height: cb_video_encoder.height * 1.24
        width: grid.width * 1.1

        x: grid.x - (width - grid.width)/2
        y: y_video_encoder
        z: 1

        PropertyAnimation {
            id: focus_indicator_animation
            properties: "y"
            target: focus_indicator
            easing.period: .3
            easing.type: Easing.InQuint
            from: focus_indicator.y
            to: {
                switch(focus_index) {
                case field_id.video_encoder:
                    return focus_indicator.y_video_encoder

                case field_id.pixel_format:
                    return focus_indicator.y_pixel_format

                case field_id.crf:
                    return focus_indicator.y_crf

                case field_id.half_fps:
                    return focus_indicator.y_half_fps

                case field_id.stop_on_drop:
                    return focus_indicator.y_stop_on_drop
                }

                return focus_indicator.y_video_enco
            }

            duration: 200
        }
    }

    //

    GridLayout {
        id: grid

        width: parent.width*.78

        z: 2

        columns: 2
        rowSpacing: settings.height*.04

        anchors.centerIn: parent

        //

        Text {
            id: l_video_encoder
            font.pixelSize: (settings.width + settings.height)/2*.04
            anchors.verticalCenter: cb_video_encoder.verticalCenter
            color: "white"
            text: "Video encoder:"
        }

        ComboBox {
            id: cb_video_encoder

            Layout.fillHeight: true
            Layout.fillWidth: true

            font.pixelSize: (settings.width + settings.height)/2*.04
            model: messenger.modelVideoEncoder

            onCurrentIndexChanged: messenger.videoCodecIndexChanged(currentIndex)

            Connections {
                target: messenger
                onVideoEncoderIndexSet: cb_video_encoder.currentIndex=index
            }
        }

        //

        Text {
            id: l_pixel_format
            font.pixelSize: (settings.width + settings.height)/2*.04
            anchors.verticalCenter: cb_pixel_format.verticalCenter
            color: "white"
            text: "Pixel format:"
        }

        ComboBox {
            id: cb_pixel_format

            Layout.fillHeight: true
            Layout.fillWidth: true

            font.pixelSize: (settings.width + settings.height)/2*.04
            model: messenger.modelPixelFormat

            onCurrentIndexChanged: messenger.pixelFormatIndexChanged(currentIndex)

            Connections {
                target: messenger
                onPixelFormatIndexSet: cb_pixel_format.currentIndex=index
            }
        }

        //

        Text {
            id: l_crf
            font.pixelSize: (settings.width + settings.height)/2*.04
            anchors.verticalCenter: cb_crf.verticalCenter
            color: "white"
            text: "Constant rate factor / quality:"
        }

        ComboBox {
            id: cb_crf

            Layout.fillHeight: true
            Layout.fillWidth: true

            font.pixelSize: (settings.width + settings.height)/2*.04
            model: [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 ]

            onCurrentIndexChanged: messenger.crfChanged(currentIndex)

            Connections {
                target: messenger
                onCrfSet: cb_crf.currentIndex=value
            }
        }

        //

        Text {
            id: l_half_fps
            font.pixelSize: (settings.width + settings.height)/2*.04
            anchors.verticalCenter: cb_half_fps.verticalCenter
            color: "white"
            text: "Half-fps:"
        }

        CheckBox {
            id: cb_half_fps

            Layout.fillHeight: true
            Layout.fillWidth: true

            font.pixelSize: (settings.width + settings.height)/2*.04
            scale: cb_crf.height*.02
            onCheckStateChanged: messenger.halfFpsChanged(checked)

            Connections {
                target: messenger
                onHalfFpsSet: cb_half_fps.checked=value
            }
        }

        //

        Text {
            id: l_stop_on_drop
            font.pixelSize: (settings.width + settings.height)/2*.04
            anchors.verticalCenter: cb_stop_on_drop.verticalCenter
            color: "white"
            text: "Stop rec on frames drop:"
        }

        CheckBox {
            id: cb_stop_on_drop

            Layout.fillHeight: true
            Layout.fillWidth: true

            font.pixelSize: (settings.width + settings.height)/2*.04
            scale: cb_crf.height*.02
            onCheckStateChanged: messenger.stopOnDropChanged(checked)

            Connections {
                target: messenger
                onStopOnDropSet: cb_stop_on_drop.checked=value
            }
        }
    }

    //

    Connections {
        target: messenger

        onBack: settings.state_visible=false

        onKeyPressed: {
            if(!state_visible)
                return

            switch(key) {
            case Qt.Key_Up:
                focusPrev()
                break

            case Qt.Key_Down:
                focusNext()
                break

            case Qt.Key_Left:
                valuePrev()
                break

            case Qt.Key_Right:
                valueNext()
                break

            default:
                break
            }
        }
    }

    //

    function focusNext() {
        settings.focus_index++

        if(settings.focus_index>field_id.max)
            settings.focus_index=field_id.video_encoder

        focus_indicator_animation.start()
    }

    function focusPrev() {
        settings.focus_index--

        if(settings.focus_index<0)
            settings.focus_index=field_id.stop_on_drop

         focus_indicator_animation.start()
    }

    function valueNext() {
        var tmp=0

        switch(settings.focus_index) {
        case field_id.video_encoder:
            tmp=cb_video_encoder.currentIndex + 1

            if(tmp>=cb_video_encoder.count)
                tmp=0

            cb_video_encoder.currentIndex=tmp

            break

        case field_id.pixel_format:
            tmp=cb_pixel_format.currentIndex + 1

            if(tmp>=cb_pixel_format.count)
                tmp=0

            cb_pixel_format.currentIndex=tmp

            break

        case field_id.crf:
            tmp=cb_crf.currentIndex + 1

            if(tmp>=cb_crf.count)
                tmp=0

            cb_crf.currentIndex=tmp

            break

        case field_id.half_fps:
            cb_half_fps.checked=!cb_half_fps.checked
            break

        case field_id.stop_on_drop:
            cb_stop_on_drop.checked=!cb_stop_on_drop.checked
            break
        }

    }

    function valuePrev() {
        var tmp=0

        switch(settings.focus_index) {
        case field_id.video_encoder:
            tmp=cb_video_encoder.currentIndex - 1

            if(tmp<0)
                tmp=cb_video_encoder.count - 1

            cb_video_encoder.currentIndex=tmp

            break

        case field_id.pixel_format:
            tmp=cb_pixel_format.currentIndex - 1

            if(tmp<0)
                tmp=cb_pixel_format.count - 1

            cb_pixel_format.currentIndex=tmp

            break

        case field_id.crf:
            tmp=cb_crf.currentIndex - 1

            if(tmp<0)
                tmp=cb_crf.count - 1

            cb_crf.currentIndex=tmp

            break

        case field_id.half_fps:
            cb_half_fps.checked=!cb_half_fps.checked
            break

        case field_id.stop_on_drop:
            cb_stop_on_drop.checked=!cb_stop_on_drop.checked
            break
        }
    }
}
