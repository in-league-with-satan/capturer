import QtQuick 2.7
import QtQml.Models 2.2
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import Qt.labs.folderlistmodel 1.0


Rectangle {
    id: root

    property int item_angle: 60
    property int item_width: height*2
    property int item_height: height
    property alias model: view.model

    color : "transparent"

    clip: true

    function next() {
        if(view.count<1)
            return

        var index=view.currentIndex + 1

        if(index>view.count)
            index=0

        if(view.currentIndex!=index)
            view.currentIndex=index
    }

    PathView {
        id: view

        anchors.fill: parent
        pathItemCount: 2

        movementDirection: (view.count>2 ? PathView.Positive : PathView.Negative)

        preferredHighlightBegin: .5
        preferredHighlightEnd: .5
        highlightRangeMode: PathView.StrictlyEnforceRange

        path: Path {
            startX: 0
            startY: height / 2

            PathPercent { value: .0 }
            PathAttribute { name: "z"; value: 0 }
            PathAttribute { name: "angle"; value: root.item_angle }
            PathAttribute { name: "origin"; value: 0 }
            PathAttribute { name: "opacity"; value: -.5 }
            PathLine {
                x: (view.width - root.item_width)/2
                y: view.height/2
            }

            PathAttribute { name: "angle"; value: root.item_angle }
            PathAttribute { name: "origin"; value: 0 }
            PathPercent { value: .4 }
            PathAttribute { name: "z"; value: 10 }
            PathLine { relativeX: 0; relativeY: 0 }

            PathLine {
                x: (view.width - root.item_width) / 2 + root.item_width
                y: view.height / 2
            }

            PathAttribute { name: "opacity"; value: 2 }
            PathPercent { value: .6 }
            PathLine { relativeX: 0; relativeY: 0 }

            PathAttribute { name: "z"; value: 10 }
            PathAttribute { name: "angle"; value: -root.item_angle }
            PathAttribute { name: "origin"; value: root.item_width }
            PathLine {
                x: view.width
                y: view.height / 2
            }

            PathPercent { value: 1 }
            PathAttribute { name: "z"; value: 0 }
            PathAttribute { name: "angle"; value: -root.item_angle }
            PathAttribute { name: "origin"; value: root.item_width }
            PathAttribute { name: "opacity"; value: -.5 }
        }

        delegate: Rectangle {
            id: convas

            property real rotation_angle: PathView.angle ? PathView.angle : 0
            property real rotation_origin: PathView.origin ? PathView.origin : 0

            color : "transparent"

            width: root.item_width
            height: root.item_height
            z: PathView.z ? PathView.z : 0

            opacity: PathView.opacity ? PathView.opacity : 0

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: "image://fs_image_provider/" + model.id
            }

            transform: Rotation {
                axis { x: 0; y: 1; z: 0 }
                angle: rotation_angle
                origin.x: rotation_origin
                origin.y: height*.5
            }
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                view.currentIndex++

                if(view.currentIndex>view.count)
                    view.currentIndex=0

                // console.log(view.currentIndex)
            }
        }
    }

    // Rectangle {
    //     anchors.fill: parent
    //     color: "#33ff0000"
    // }
}
