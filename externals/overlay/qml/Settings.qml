import QtQuick 2.7
import QtQuick.Window 2.2
//import QtQuick.Controls 1.5
import QtQuick.Controls 2.1


Rectangle {
    id: settings

    color: "#bb000000"

    property bool state_visible: false
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

        property int new_y: grid.y + l_video_encoder.y - l_video_encoder.height/3.5

        height: cb_video_encoder.height * 1.4
        width: grid.width * 1.1

        x: grid.x - (width - grid.width)/2
        y: grid.y + l_video_encoder.y - l_video_encoder.height/3.5
        z: 1

        PropertyAnimation {
            id: focus_indicator_animation
            properties: "y"
            target: focus_indicator;
            easing.period: 0.3
            easing.type: Easing.InQuint
            from: focus_indicator.y;
            to: focus_indicator.new_y;
            duration: 100
            running: true
        }
    }

    //

    Grid {
        id: grid

        z: 2

        columns: 2
        // spacing: Screen.height*.04
        rowSpacing: Screen.height*.02
        columnSpacing: Screen.height*.02

        anchors.centerIn: parent

        //

        Text {
            id: l_video_encoder
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Video encoder:"
        }

        ComboBox {
            id: cb_video_encoder
            width: settings.width*.4

            font.pixelSize: Screen.height*.04
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
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Pixel format:"
        }


        ComboBox {
            id: cb_pixel_format
            width: settings.width*.4

            font.pixelSize: Screen.height*.04
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
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Constant rate factor / quality:"
        }

        ComboBox {
            id: cb_crf
            width: settings.width*.4

            font.pixelSize: Screen.height*.04
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
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Half-fps:"
        }

        CheckBox {
            id: cb_half_fps
            width: settings.width*.4

            font.pixelSize: Screen.height*.04
            scale: parent.height*.003
            onCheckStateChanged: messenger.halfFpsChanged(checked)

            Connections {
                target: messenger
                onHalfFpsSet: cb_half_fps.checked=value
            }
        }

        //

        Text {
            id: l_stop_on_drop
            font.pixelSize: Screen.height*.04
            color: "white"
            text: "Stop rec on frames drop:"
        }

        CheckBox {
            id: cb_stop_on_drop
            width: settings.width*.4
            font.pixelSize: Screen.height*.04
            scale: parent.height*.003
            onCheckStateChanged: messenger.stopOnDropChanged(checked)

            Connections {
                target: messenger
                onStopOnDropSet: cb_stop_on_drop.checked=value
            }
        }
    }

    //

    states: [
        State {
            when: state_visible

            PropertyChanges {
                target: settings
                opacity: 1.
            }
        },

        State {
            when: !state_visible

            PropertyChanges {
                target: settings
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

        setFocus(settings.focus_index)
    }

    function focusPrev() {
        settings.focus_index--

        if(settings.focus_index<0)
            settings.focus_index=field_id.stop_on_drop

        setFocus(settings.focus_index)
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

    function setFocus(focus_index) {
        switch(focus_index) {
        case field_id.video_encoder:
            focus_indicator.new_y=grid.y + l_video_encoder.y - l_video_encoder.height/3.5
            break

        case field_id.pixel_format:
            focus_indicator.new_y=grid.y + l_pixel_format.y - l_pixel_format.height/3.5
            break

        case field_id.crf:
            focus_indicator.new_y=grid.y + l_crf.y - l_crf.height/3.5
            break

        case field_id.half_fps:
            focus_indicator.new_y=grid.y + l_half_fps.y - l_half_fps.height/3.5
            break

        case field_id.stop_on_drop:
            focus_indicator.new_y=grid.y + l_stop_on_drop.y - l_stop_on_drop.height/3.5
            break
        }

        focus_indicator_animation.start()
    }
}

