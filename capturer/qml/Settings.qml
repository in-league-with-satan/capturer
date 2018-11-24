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
import QtQuick.Controls 2.1

import FuckTheSystem 0.0


ShowHideRect {
    id: root
    color: "#bb000000"
    clip: true

    ListView {
        id: list
        anchors.fill: parent
        cacheBuffer: height*4

        currentIndex: 1

        highlight: Rectangle {
            color: "#aa3355ff"
            radius: root.height*.017
        }

        highlightMoveDuration: 200
        highlightResizeDuration: 200
        preferredHighlightBegin: height/2 - (currentItem ? currentItem.height*.5 : 0)
        preferredHighlightEnd: height/2 + (currentItem ? currentItem.height*.5 : 0)
        highlightRangeMode: ListView.ApplyRange

/*
        section.property: "group"
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels
        section.delegate: Rectangle {
            width: root.width
            height: root.height*.1
            color: "transparent"
            Text {
                text: section
                color: "white"
                font.pixelSize: parent.height*.5
                anchors.centerIn: parent
                font.bold: true
            }
        }

        onWidthChanged: messenger.settingsModel.reload()
        onHeightChanged: messenger.settingsModel.reload()
*/

        property int role_type: 0
        property int role_group: 1
        property int role_priority: 2
        property int role_values: 3
        property int role_values_data: 4
        property int role_value: 5
        property int role_name: 6


        property int type_title: 0
        property int type_divider: 1
        property int type_combobox: 2
        property int type_checkbox: 3
        property int type_button: 4


        onVisibleChanged: {
            if(!posCheck(list.currentIndex))
                focusNext()
        }

        model: VisualDataModel {
            id: fs_model
            model: messenger.settingsModel

            delegate: Rectangle {
                id: m_delegate
                width: root.width
                height: {
                    if(item_type==list.type_divider)
                        return root.height*.1*.5

                    return root.height*.1
                }
                color: "transparent"

                property int item_type: type
                property int item_value: value
                property var item_values: values
                property string item_name: name

                property alias combobox: combobox

                function valuePrev() {
                    if(item_type==list.type_combobox) {
                        var index=combobox.currentIndex - 1

                        if(index<0)
                            index=combobox.count>0 ? combobox.count - 1 : 0

                        combobox.currentIndex=index

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, index, true)

                        return
                    }

                    if(item_type==list.type_checkbox) {
                        checkbox.checked=!checkbox.checked

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, checkbox.checked, true)

                        return
                    }

                    if(item_type==list.type_button) {
                        messenger.settingsModel.setData(list.currentIndex, list.role_value, 0, true)

                        return
                    }
                }

                function valueNext() {
                    if(item_type==list.type_combobox) {
                        var index=combobox.currentIndex + 1

                        if(index>=combobox.count)
                            index=0

                        combobox.currentIndex=index

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, index, true)

                        return
                    }

                    if(item_type==list.type_checkbox) {
                        checkbox.checked=!checkbox.checked

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, checkbox.checked, true)

                        return
                    }

                    if(item_type==list.type_button) {
                        messenger.settingsModel.setData(list.currentIndex, list.role_value, 0, true)

                        return
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height*.33
                    color: {
                        if(item_type==list.type_divider)
                            return "#ff323232"

                        return "transparent"
                    }

                    z: parent.z - 1
                }

                Text {
                    id: t_name
                    color: "white"
                    width: {
                        if(item_type==list.type_title)
                            return m_delegate.width

                        return m_delegate.width*.5
                    }
                    height: m_delegate.height
                    anchors.verticalCenter: m_delegate.verticalCenter
                    font.pixelSize: m_delegate.height*.5
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: {
                        if(item_type==list.type_title)
                            return Text.AlignHCenter

                        return Text.AlignLeft
                    }
                    clip: true
                    text: {
                        if(item_type==list.type_title)
                            return item_name

                        if(item_type==list.type_divider)
                            return ""

                        if(item_type==list.type_button)
                            return ""

                        if(item_name!="")
                            return "  " + item_name + ":"

                        return ""
                    }

                    /*
                    Rectangle {
                        anchors.fill: parent
                        color: "#4400ff00"
                        z: parent.z - 1
                    }
                    */
                }

                Rectangle {
                    width: m_delegate.width*.48
                    height: m_delegate.height*.8
                    anchors.left: t_name.right
                    anchors.verticalCenter: m_delegate.verticalCenter
                    color : "transparent"

                    ComboBox {
                        id: combobox
                        visible: m_delegate.item_type==list.type_combobox
                        model: item_values
                        font.pixelSize: m_delegate.height*.5
                        anchors.fill: parent

                        currentIndex: {
                            if(m_delegate.item_type==list.type_combobox)
                                return m_delegate.item_value

                            return 0
                        }
                    }

                    CheckBox {
                        id: checkbox
                        visible: m_delegate.item_type==list.type_checkbox
                        anchors.fill: parent
                        scale: m_delegate.height*.02

                        checked: {
                            if(m_delegate.item_type==list.type_checkbox)
                                return m_delegate.item_value

                            return false
                        }
                    }

                    Button {
                        id: button
                        visible: m_delegate.item_type==list.type_button
                        anchors.fill: parent
                        font.pixelSize: m_delegate.height*.5
                        text: {
                            if(m_delegate.item_type==list.type_button)
                                return m_delegate.item_name

                            return ""
                        }

                        contentItem: Text {
                            text: button.text
                            font: button.font
                            color: "#333435"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        background: Rectangle {
                            color: "#e0e0e0"
                            // border.color: "black"
                            // border.width: root.height*.002
                            radius: root.height*.017
                        }
                    }
                }
            }
        }

        function posCheck(index) {
            if(index<0)
                return false

            if(index>=list.model.count)
                return false

            if(!list.contentItem.children[index])
                return false

            if(list.contentItem.children[index].item_type===list.type_title
                    || list.contentItem.children[index].item_type===list.type_divider) {
                return false
            }

            return true
        }


        function focusPrev() {
            if(!root.state_visible)
                return

            list.currentIndex=messenger.settingsModel.focusPrev(list.currentIndex)
        }

        function focusNext() {
            if(!root.state_visible)
                return

            list.currentIndex=messenger.settingsModel.focusNext(list.currentIndex)
        }

        function valuePrev() {
            list.currentItem.valuePrev()
        }

        function valueNext() {
            list.currentItem.valueNext()
        }

        Connections {
            target: messenger

            onKeyPressed: {
                if(!root.state_visible)
                    return

                switch(key) {
                case Qt.Key_Up:
                    list.focusPrev()
                    break

                case Qt.Key_Down:
                    list.focusNext()
                    break

                case Qt.Key_Left:
                    list.valuePrev()
                    break

                case Qt.Key_Right:
                    list.valueNext()
                    break

                default:
                    break
                }
            }
        }
    }
}
