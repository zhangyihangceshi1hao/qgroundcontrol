import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

GridLayout {
    columns:        2
    rowSpacing:     _rowSpacing
    columnSpacing:  _colSpacing

    function saveSettings() {
        subEditConfig.host = hostField.text
        subEditConfig.deviceId = deviceIdField.text
    }

    /**
      * MQTTX公用服务器测试： broker.emqx.io
      * MQTXX 下载地址：https://mqttx.app/zh
      */
    QGCLabel { text: qsTr("服务器") }
    QGCTextField {
        id:                     hostField
        Layout.preferredWidth:  _secondColumnWidth
        text:                   subEditConfig && subEditConfig.linkType === LinkConfiguration.TypeMqtt
                                ? subEditConfig.host : ""
    }

    /**
     * 每个连接的 MQTT 客户端都需要提供一个唯一的 Client ID，以区分不同的客户端。
     * 如果多个客户端使用相同的 Client ID，MQTT 代理会断开先前的连接，保留最新的客户端连接。
     */
    QGCLabel { text: qsTr("客户端ID") }
    QGCTextField {
        id:                     deviceIdField
        Layout.preferredWidth:  _secondColumnWidth
        text:                   subEditConfig && subEditConfig.linkType === LinkConfiguration.TypeMqtt
                                ? subEditConfig.deviceId : ""
        inputMethodHints:       Qt.ImhFormattedNumbersOnly
    }
}

