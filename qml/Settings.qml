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
        property int role_values: 2
        property int role_values_data: 3
        property int role_value: 4
        property int role_name: 5

        property int type_combobox: 0
        property int type_checkbox: 1
        property int type_value: 2

        model: VisualDataModel {
            id: fs_model
            model: messenger.settingsModel

            delegate: Rectangle {
                id: m_delegate
                width: root.width
                height: root.height*.1
                color : "transparent"

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
                    }

                    if(item_type==list.type_checkbox) {
                        checkbox.checked=!checkbox.checked

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, checkbox.checked, true)
                    }
                }

                function valueNext() {
                    if(item_type==list.type_combobox) {
                        var index=combobox.currentIndex + 1

                        if(index>=combobox.count)
                            index=0

                        combobox.currentIndex=index

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, index, true)
                    }

                    if(item_type==list.type_checkbox) {
                        checkbox.checked=!checkbox.checked

                        messenger.settingsModel.setData(list.currentIndex, list.role_value, checkbox.checked, true)
                    }
                }

                Text {
                    id: t_name
                    color: "white"
                    width: m_delegate.width*.5
                    height: m_delegate.height
                    anchors.verticalCenter: m_delegate.verticalCenter
                    font.pixelSize: m_delegate.height*.5
                    verticalAlignment: Text.AlignVCenter
                    clip: true
                    text: "  " + item_name + ":"

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "#4400ff00"
                    //     z: parent.z - 1
                    // }
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
                        anchors.fill: parent;

                        currentIndex: {
                            if(m_delegate.item_type==list.type_combobox)
                                return m_delegate.item_value

                            return 0
                        }
                    }

                    CheckBox {
                        id: checkbox
                        visible: m_delegate.item_type==list.type_checkbox
                        anchors.fill: parent;
                        scale: m_delegate.height*.02

                        checked: {
                            if(m_delegate.item_type==list.type_checkbox)
                                return m_delegate.item_value

                            return false
                        }
                    }
                }
            }
        }

        function focusPrev() {
            if(!root.state_visible)
                return;

            list.currentIndex--;

            if(list.currentIndex<0)
                list.currentIndex=list.model.count - 1;
        }

        function focusNext() {
            if(!root.state_visible)
                return;

            list.currentIndex++;

            if(list.currentIndex>=list.model.count)
                list.currentIndex=0
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

        Connections {
            target: messenger.settingsModel

            onDataChanged: {
                if(role==list.role_values)
                    list.contentItem.children[row].combobox.model=messenger.settingsModel.data(row, list.role_values)

                if(!qml && role==list.role_value)
                    list.contentItem.children[row].combobox.currentIndex=messenger.settingsModel.data(row, list.role_value)
            }
        }
    }
}
