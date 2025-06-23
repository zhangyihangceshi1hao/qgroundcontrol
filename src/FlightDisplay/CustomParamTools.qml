import QtQuick                  2.3
import QtQuick.Controls         1.2
import QtQuick.Dialogs          1.2
import QtPositioning            5.3

import QGroundControl                           1.0
import QGroundControl.ScreenTools               1.0
import QGroundControl.Controls                  1.0
import QGroundControl.Palette                   1.0
import QGroundControl.Vehicle                   1.0

Rectangle {
    id:         root
    width:      dataItem.width + _margin * 4
    height:     dataItem.height + _margin * 3
    color:      Qt.rgba(0, 0, 0, 0.8)   // 背景颜色
    radius:     3

    property var    _activeVehicle:                 QGroundControl.multiVehicleManager.activeVehicle

    property real   _rollAngle:                     _activeVehicle ? _activeVehicle.roll.rawValue  : 0
    property real   _pitchAngle:                    _activeVehicle ? _activeVehicle.pitch.rawValue : 0
    property real   _headingAngle:                  _activeVehicle ? _activeVehicle.heading.rawValue : 0
    property real   _altitudeRelative:              _activeVehicle ? _activeVehicle.altitudeRelative.rawValue : 0
    property real   _altitudeAMSL:                  _activeVehicle ? _activeVehicle.altitudeAMSL.rawValue : 0
    property real   _altitudeAll:                   _activeVehicle ? _altitudeRelative + _altitudeAMSL : 0
    property real   _groundSpeed:                   _activeVehicle ? _activeVehicle.groundSpeed.rawValue : 0
    property real   _climbSpeed:                    _activeVehicle ? _activeVehicle.climbRate.rawValue : 0
    property var    activeVehicleCoordinate:        _activeVehicle ? _activeVehicle.coordinate : QtPositioning.coordinate()

    property color _nameColor:                      "#FFFFFF"
    property color _valueColor:                     "#FFFFFF"
    property real  _nameWidth:                      ScreenTools.defaultFontPixelWidth * 4
    property real  _valueWidth:                     ScreenTools.defaultFontPixelWidth * 9
    property real  _margin:                         ScreenTools.defaultFontPixelWidth

    MouseArea {
        anchors.fill: parent
        onClicked: {

        }
    }

    // *************** 参数区域 *****************
    Item {
        id:             dataItem
        height:         _dataColumn.height
        width:          _dataColumn.width
        anchors.centerIn: parent

        Column {
            id:         _dataColumn
            spacing:    _margin

            Row {
                spacing:    _margin * 2

                // 速度
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "速度"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           (_activeVehicle ? _groundSpeed.toFixed(1)  : "--.--") + "m/s"
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }

                // 高度
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "高度"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           (_activeVehicle ? _altitudeRelative.toFixed(1) : "--.--") + "m"
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }

                // 航程
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "航程"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           (_activeVehicle ? (_activeVehicle.flightDistance.value / 1000.0).toFixed(3) : "--.--") + "m"
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }
            }

            Row {
                spacing:    _margin * 2

                // 经度
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "经度"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           isNaN(activeVehicleCoordinate.longitude) ? "--.--" : activeVehicleCoordinate.longitude.toFixed(6)
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }

                // 纬度
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "纬度"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           isNaN(activeVehicleCoordinate.latitude) ? "--.--" : activeVehicleCoordinate.latitude.toFixed(6)
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }

                // GPS状态
                Row {
                    spacing:                            _margin

                    QGCLabel {
                        text:                           "GPS"
                        font.bold:                      true
                        color:                          _nameColor
                        anchors.verticalCenter:         parent.verticalCenter
                        width:                          _nameWidth
                    }

                    QGCLabel {
                        font.bold:                      true
                        text:                           _activeVehicle ? _activeVehicle.gps.lock.enumStringValue : "--.--"
                        color:                          _valueColor
                        width:                          _valueWidth
                        anchors.verticalCenter:         parent.verticalCenter
                    }
                }
            }
        }
    }
}

