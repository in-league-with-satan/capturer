/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

import QtQuick 2.7
import QtQml.Models 2.2
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import Qt.labs.folderlistmodel 1.0

import FuckTheSystem 0.0


ShowHideRect {
    id: root
    color: "#bbbbbbbb"
    clip: true

    ListView {
        id: list

        anchors.fill: parent

        cacheBuffer: height*4

        highlight: Rectangle {
            color: "lightsteelblue"
            // radius: 5
        }

        highlightMoveDuration: 200
        highlightResizeDuration: 200

        preferredHighlightBegin: height/2 - (currentItem ? currentItem.height*.5 : 0)
        preferredHighlightEnd: height/2 + (currentItem ? currentItem.height*.5 : 0)
        highlightRangeMode: ListView.ApplyRange

        Component.onCompleted: {
            fs_model.rootIndex=messenger.fileSystemModel.rootPathIndex()
            fs_model.file_path_last=messenger.fileSystemModel.rootPath()
        }

        model: VisualDataModel {
            id: fs_model

            model: messenger.fileSystemModel

            property string file_path_last: ""

            onRootIndexChanged: {
                if(file_path_last!="")
                    fs_model.rootIndex=messenger.fileSystemModel.index(file_path_last)
            }

            delegate: Rectangle {
                id: fs_delegate

                width: list.width
                height: base_height + base_height*.1 + (t_media_info.paintedHeight>cover_flow.height ? t_media_info.paintedHeight : cover_flow.height)

                color : "transparent"

                property string file_path: filePath
                property string file_name: fileName
                property string file_size: messenger.fileSystemModel.fileSize(fs_model.modelIndex(index))
                property bool is_dir: messenger.fileSystemModel.isDir(fs_model.modelIndex(index))
                property string ext: messenger.fileSystemModel.ext(fs_model.modelIndex(index))
                property string media_info: messenger.fileSystemModel.mediaInfo(fs_model.modelIndex(index))


                property int base_height: list.height*.08
                property real font_size: base_height*.7

                Image {
                    id: icon
                    x: base_height*.5
                    y: base_height*.1
                    width: base_height*2
                    height: base_height
                    sourceSize.width: width
                    sourceSize.height: height
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignHCenter
                    verticalAlignment: Image.AlignVCenter
                    source: is_dir ? "qrc:/images/folder.svg" : "qrc:/images/mkv.svg"
                }

                Text {
                    id: t_file_name
                    width: (fs_delegate.width - icon.width)*.7
                    height: base_height
                    anchors.left: icon.right
                    font.pixelSize: fs_delegate.font_size
                    text: file_name

                    clip: true
                    elide: Text.ElideRight

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "#4400ff00"
                    //     z: parent.z - 1
                    // }
                }

                Text {
                    id: t_file_size
                    width: (fs_delegate.width - icon.width*4)*.3
                    height: base_height
                    anchors.left: t_file_name.right
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: fs_delegate.font_size
                    // wrapMode: "WordWrap"
                    text: is_dir ? "" : file_size

                    clip: true
                    elide: Text.ElideLeft

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "#44ff0000"
                    // }
                }

                Text {
                    id: t_media_info
                    width: (fs_delegate.width - icon.width)*.35
                    anchors.left: icon.right
                    anchors.top: t_file_name.bottom
                    font.pixelSize: text=="" ? 0 : fs_delegate.font_size*.5
                    text: media_info

                    clip: true
                    elide: Text.ElideRight
                }

                CoverFlow {
                    id: cover_flow

                    width: (fs_delegate.width - icon.width)*.65 - base_height*.5
                    height: t_media_info.paintedHeight

                    anchors.left: t_media_info.right
                    anchors.top: t_file_name.bottom

                    model: messenger.fileSystemModel.snapshotListModel(fs_model.modelIndex(index))

                    Timer {
                        interval: 2000
                        repeat: true
                        running: true
                        onTriggered: cover_flow.next()
                    }

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "#44ff00ff"
                    // }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: {
                        if(messenger.fileSystemModel.isDir(file_path)) {
                            fs_model.file_path_last=file_path
                            fs_model.rootIndex=messenger.fileSystemModel.index(file_path)
                        }
                    }
                }
            }
        }

        function focusPrev() {
            if(!root.state_visible)
                return

            list.currentIndex--

            if(list.currentIndex<0)
                list.currentIndex=list.model.count - 1
        }

        function focusNext() {
            if(!root.state_visible)
                return

            list.currentIndex++

            if(list.currentIndex>=list.model.count)
                list.currentIndex=0
        }

        function back() {
            if(!root.state_visible)
                return

            var path=messenger.fileSystemModel.path(list.model.modelIndex(0))

            if(path.endsWith("..") && messenger.fileSystemModel.isDir(path)) {
                fs_model.file_path_last=path
                fs_model.rootIndex=messenger.fileSystemModel.index(path)
            }
        }

        function enter() {
            if(!root.state_visible)
                return

            var path=messenger.fileSystemModel.path(list.model.modelIndex(list.currentIndex))

            if(messenger.fileSystemModel.isDir(path)) {
                fs_model.file_path_last=path
                fs_model.rootIndex=messenger.fileSystemModel.index(path)

            } else {
                messenger.fileSystemModel.playMedia(path)

                root.state_visible=false
            }
        }

        onVisibleChanged: {
            messenger.fileBrowserVisibleState(visible)
        }

        Connections {
            target: messenger.fileSystemModel

        }

        Connections {
            target: messenger

            onKeyPressed: {
                switch(key) {
                case Qt.Key_Up:
                    list.focusPrev()
                    break

                case Qt.Key_Down:
                    list.focusNext()
                    break

                case Qt.Key_Left:
                    list.back()
                    break

                case Qt.Key_Right:
                    list.enter()
                    break

                default:
                    break
                }
            }
        }
    }
}
