#ifndef MQTTLINK_H
#define MQTTLINK_H

#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QHostAddress>
#include <LinkInterface.h>

#include "qmqtt/qmqttclient.h"

// Even though QAbstractSocket::SocketError is used in a signal by Qt, Qt doesn't declare it as a meta type.
// This in turn causes debug output to be kicked out about not being able to queue the signal. We declare it
// as a meta type to silence that.
#include <QMetaType>

class MqttConfiguration;
class LinkManager;

/**
 * @brief 基于 https://github.com/emqx/qmqtt 模块二次发开
 */
class MqttConfiguration : public LinkConfiguration
{
    Q_OBJECT

public:
    Q_PROPERTY(QString  host        READ  host      WRITE setHost       NOTIFY hostChanged)
    Q_PROPERTY(QString  deviceId    READ  deviceId  WRITE setDeviceId   NOTIFY deviceIdChanged)

    MqttConfiguration(const QString& name);
    MqttConfiguration(MqttConfiguration* source);

    // 设置MQTT服务器地址，并自动解析为IP
    const QString       host      () { return _host; }
    void setHost    (const QString host);

    // 获取设备ID（用作ClientId）
    QString deviceId   () { return _deviceId; }
    void setDeviceId   (const QString deviceId);

    //LinkConfiguration overrides
    LinkType    type            () override { return LinkConfiguration::TypeMqtt; }
    void        copyFrom        (LinkConfiguration* source) override;
    void        loadSettings    (QSettings& settings, const QString& root) override;
    void        saveSettings    (QSettings& settings, const QString& root) override;
    QString     settingsURL     () override { return "MqttSettings.qml"; } ;
    QString     settingsTitle   (void) override  { return tr("Mqtt Link Settings"); };

signals:
    void hostChanged();
    void deviceIdChanged();

private:
    QString _host;
    QString _deviceId;
};

class MqttLink : public LinkInterface
{
    Q_OBJECT

public:
    MqttLink(SharedLinkConfigurationPtr& config);
    ~MqttLink();

    //QMqttSocket* getSocket(void) { return _socket; }
    void signalBytesWritten(void);

    // LinkInterface overrides
    bool isConnected(void) const override;  // 检查MQTT连接是否仍然有效
    void disconnect(void) override;         // 断开MQTT连接

    // QThread overrides
    void run(void) override;                // 启动线程，执行连接与事件循环

private slots:
    // From LinkInterface
    void _writeBytes(const QByteArray data) override;

protected slots:
    //void _socketError(QAbstractSocket::SocketError socketError);

    // From LinkInterface
    virtual void readBytes(QByteArray msg);

private:
    // From LinkInterface
    bool _connect(void) override;
    bool _hardwareConnect();

    MqttConfiguration* _MqttConfig;
    QMQTTClient*       _mqttClient;
    bool               _isConnected;
};

#endif // MqttLINK_H
