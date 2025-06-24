/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0
import QGroundControl.Vehicle       1.0
import QGroundControl.FlightMap     1.0

Item {
    id: root
    property real   _margin:            ScreenTools.defaultFontPixelWidth / 2
    property real   _widgetHeight:      ScreenTools.defaultFontPixelHeight * 4
    property color  _textColor:         "white" //"black"
    property real   _rectOpacity:       0.8
    property var    _guidedController:  globals.guidedControllerFlyView

    property real _valueHeight: ScreenTools.defaultFontPixelHeight * 1.3

    property color _bgColor: Qt.rgba(0, 0, 0, 0.8)//"#cc262626"

    QGCPalette { id: qgcPal }

    // 控制所有设备命名（解锁/上锁、开始任务、暂停任务）
    Rectangle {
        id:             mvCommands
        anchors.left:   parent.left
        anchors.right:  parent.right
        height:         mvCommandsColumn.height + (_margin *2)
        color:          _bgColor //Qt.rgba(0, 0, 0, 0.7)
        //   color:          qgcPal.missionItemEditor
        //  opacity:        _rectOpacity
        radius:         _margin
        //visible:        false

        DeadMouseArea {
            anchors.fill: parent
        }

        Column {
            id:                 mvCommandsColumn
            anchors.margins:    _margin
            anchors.top:        parent.top
            anchors.left:       parent.left
            anchors.right:      parent.right
            spacing:            _margin

            QGCLabel {
                anchors.left:   parent.left
                anchors.right:  parent.right
                text:           qsTr("The following commands will be applied to all vehicles")
                color:          _textColor
                wrapMode:       Text.WordWrap
                font.pointSize: ScreenTools.smallFontPointSize
                visible:        false
            }

            Row {
                spacing:            _margin

                // 解锁/上锁所有设备
                QGCButton {
                    id:         armButtom
                    text:       QGroundControl.multiVehicleManager.vehicles.count > 0 ? (QGroundControl.multiVehicleManager.vehicles.get(0).armed ? "上锁所有设备" : "解锁所有设备") : ""
                    onClicked:  {
                        armDialog.open()
                    }

                    MessageDialog {
                        id:                 armDialog
                        title:              armButtom.text
                        text:               "确定" + armButtom.text + "吗?"
                        standardButtons:    StandardButton.Yes | StandardButton.No
                        onYes: {
                            let rgVehicle = QGroundControl.multiVehicleManager.vehicles
                            for (var i = 0; i < rgVehicle.count; i++) {
                                rgVehicle.get(i).armed = !rgVehicle.get(i).armed
                            }
                        }
                    }
                }

                QGCButton {
                    text:       qsTr("暂停任务")
                    onClicked:  _guidedController.confirmAction(_guidedController.actionMVPause)
                }

                QGCButton {
                    text:       qsTr("开始任务")
                    onClicked:  _guidedController.confirmAction(_guidedController.actionMVStartMission)
                }
            }
        }
    }

    QGCListView {
        id:                 missionItemEditorListView
        anchors.left:       parent.left
        anchors.right:      parent.right
        anchors.topMargin:  _margin
        anchors.top:        mvCommands.visible ? mvCommands.bottom : parent.top
        anchors.bottom:     parent.bottom
        spacing:            _margin / 2
        orientation:        ListView.Vertical
        model:              QGroundControl.multiVehicleManager.vehicles
        cacheBuffer:        _cacheBuffer < 0 ? 0 : _cacheBuffer
        clip:               true

        property real _cacheBuffer:     height * 2

        delegate: Rectangle {
            height: _column.height + _margin * 2
            width:  parent.width
            color: _bgColor //"#3C4650"
            radius: _margin

            // index 0 = QGroundControl.multiVehicleManager.vehicles.get(0)
            property var    _vehicle:   object

            property var    _battery_percent: 0
            // 定时更新电池电压
            Timer {
                interval: 500; running: true; repeat: true
                onTriggered: {
                    if(_vehicle) {
                        //console.log("电池电压", _vehicle.batteries.get(0).voltage.valueString)
                        //console.log("电池电量", _vehicle.batteries.get(0).percentRemaining.valueString)
                        _battery_percent = _vehicle.batteries.get(0).percentRemaining.valueString
                    }
                }
            }

            Column {
                id:                 _column
                width:              parent.width - _margin * 4
                spacing:            _margin * 1.5
                anchors.centerIn:   parent

                // 无人机ID和电池状态
                Row {
                    width: parent.width
                    height: _perHeight
                    spacing: _margin * 2

                    QGCLabel {
                        id: _vehicleIdLabel
                        width: 60
                        text:  "ID:" + _vehicle.id
                        font.pointSize: ScreenTools.defaultFontPointSize * 1.5
                        font.bold: true
                        color: _valueColor
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // 设备位置（经纬度） + 设备电池电量
                    Column {
                        width:      parent.width - _vehicleIdLabel.width - parent.spacing
                        spacing:    _margin * 0.33
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            height: _positionLabel.height
                            width: parent.width

                            QGCLabel {
                                id: _positionLabel
                                text: "位置 " + "(" + _vehicle.coordinate.latitude.toFixed(6)
                                      + ", "  + _vehicle.coordinate.longitude.toFixed(6) + ")" //"(90.1231234, 123.1231234)"
                                color: "white"
                                anchors.left: parent.left
                            }

                            QGCLabel {
                                text: "电量 " + _battery_percent+ "%"
                                color:  "white"
                                anchors.right: parent.right
                            }
                        }

                        // 电量进度条
                        ProgressBar {
                            value: _battery_percent / 100
                            width: parent.width
                            height: 10

                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 10
                                color: "#e0e0e0"
                                radius: 5
                            }

                            contentItem: Item {
                                implicitWidth: 100
                                implicitHeight: 10

                                Rectangle {
                                    width: parent.width * parent.parent.value
                                    height: parent.height
                                    radius: 5
                                    color: _battery_percent > 70 ? "#2ecc71" : _battery_percent > 30 ? "#f39c12" : "#e74c3c"
                                }
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: infoGrid.height //parent.height - _perHeight - parent.spacing

                    // 网格布局展示颜色方块
                    // 自适应网格布局
                    Flow {
                        id: infoGrid
                        //anchors.fill: parent
                        spacing: _margin * 1.5
                        width: parent.width

                        property int itemWidth:  ScreenTools.defaultFontPixelWidth * 8   // 每个方块的基础宽度

                        // 动态创建参数项
                        InfoItem {
                            label: "离家距离"
                            value: _vehicle.distanceToHome.rawValue.toFixed(1) + " m"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "空速"
                            value: _vehicle.airSpeed.rawValue.toFixed(1) + " m/s"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "地速"
                            value: _vehicle.groundSpeed.rawValue.toFixed(1) + " m/s"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "爬升速度"
                            value: _vehicle.climbRate.rawValue.toFixed(1) + " m/s"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "高度"
                            value: _vehicle.altitudeRelative.rawValue.toFixed(1) + " m"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "俯仰角度"
                            value: _vehicle.pitch.rawValue.toFixed(1) + "°"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "航向角度"
                            value: _vehicle.heading.rawValue.toFixed(1) + "°"
                            width: infoGrid.itemWidth
                        }

                        InfoItem {
                            label: "滚转角度"
                            value: _vehicle.roll.rawValue.toFixed(1) + "°"
                            width: infoGrid.itemWidth
                        }
                    }
                }

                Item {
                    height: 1
                    width: 1
                }
            }
        }
    } // QGCListView

    property real _perHeight: ScreenTools.defaultFontPixelHeight * 2.5
    property color _nameColor: "#e0e0e0"
    property color _valueColor: "#ffffff"
    component InfoItem: Column {
        property string label
        property string value

        spacing: _margin * 0.33
        width: ScreenTools.defaultFontPixelWidth * 8

        QGCLabel {
            text: label
            font.pointSize: ScreenTools.defaultFontPointSize * 0.9
            color: _nameColor
        }

        QGCLabel {
            text: value
            font.pointSize: ScreenTools.defaultFontPointSize
            font.bold: true
            color: _valueColor
        }
    }
} // Item
