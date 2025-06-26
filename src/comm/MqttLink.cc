#include <QTimer>
#include <QList>
#include <QDebug>
#include <QMutexLocker>
#include "MqttLink.h"
#include <QHostInfo>
#include <QSignalSpy>

#include "qsslconfiguration.h"

const QString DEAFAULT_HOST = QString("192.168.1.1"); // 默认MQTT服务器地址
// const quint16 DEAFAULT_PORT = 8883;
const quint16 DEFAULT_MQTT_PORT = 1883;

/// @file
///     @brief Mqtt link type for SITL support
///
///     @author Don Gagne <don@thegagnes.com>

MqttLink::MqttLink(SharedLinkConfigurationPtr& config)
    : LinkInterface(config)
    , _MqttConfig(qobject_cast<MqttConfiguration*>(config.get()))
    , _isConnected(false)
{
    Q_ASSERT(_MqttConfig);

    moveToThread(this);
}

MqttLink::~MqttLink()
{
    disconnect();
    quit(); // Tell the thread to exit
    wait(); // Wait for it to exit
    this->deleteLater();
}

void MqttLink::run()
{
    _hardwareConnect();
    exec();
}

//--------------------------------------------------------------------------
//-- MqttConfiguration

static bool is_ip(const QString& address)
{
    int a,b,c,d;
    if (sscanf(address.toStdString().c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4
            && strcmp("::1", address.toStdString().c_str())) {
        return false;
    } else {
        return true;
    }
}

static QString get_ip_address(const QString& address)
{
    if(is_ip(address))
        return address;
    // Need to look it up
    QHostInfo info = QHostInfo::fromName(address);
    if (info.error() == QHostInfo::NoError)
    {
        QList<QHostAddress> hostAddresses = info.addresses();
        QHostAddress address;
        for (int i = 0; i < hostAddresses.size(); i++)
        {
            // Exclude all IPv6 addresses
            if (!hostAddresses.at(i).toString().contains(":"))
            {
                return hostAddresses.at(i).toString();
            }
        }
    }
    return QString("");
}

/**
 * @brief 发布数据到MQTT主题（如 testtopic/GCS）
 **/
void MqttLink::_writeBytes(const QByteArray data)
{
    if (!_mqttClient)
        return;

    _mqttClient->publishMsg(data);
    //qDebug() << "msg len:" << data.size();
}

/**
 * @brief 接收从MQTT主题订阅到的数据
 **/
void MqttLink::readBytes(QByteArray msg)
{  
    emit bytesReceived(this, msg);
    // qint64 byteCount = msg.size();

    qDebug() << "[MQTT] readBytes:" << msg;
    //qDebug() << "[接收Mqtt数据] byteCount:" << byteCount;
}

void MqttLink::disconnect(void)
{
    quit();
    wait();

    if (_mqttClient) {
        if (_mqttClient->isConnectedToHost()) {
            // qDebug() << "[MQTT] 正在断开...";
            QMetaObject::invokeMethod(_mqttClient, "disconnectFromHost", Qt::QueuedConnection);
        }
        _mqttClient->deleteLater();
        _mqttClient = nullptr;
        _isConnected = false;
        emit disconnected();
    }
}

/**
 * @brief Connect the connection.
 *
 * @return True if connection has been established, false if connection couldn't be established.
 **/
bool MqttLink::_connect(void)
{
    if (isRunning())
    {
        quit();
        wait();
    }

    start(HighPriority);
    return true;
}

bool MqttLink::_hardwareConnect()
{
    const QString hostIP = get_ip_address(_MqttConfig->host());
    if (hostIP.isEmpty()) {
        qWarning() << "Error: 服务器地址为空";
        return false;
    }

    _mqttClient = new QMQTTClient(QHostAddress(get_ip_address(_MqttConfig->host())), DEFAULT_MQTT_PORT);

    /**
     * 每个连接的 MQTT 客户端都需要提供一个唯一的 Client ID，以区分不同的客户端。
     * 如果多个客户端使用相同的 Client ID，MQTT 代理会断开先前的连接，保留最新的客户端连接。
     */
    /*
    // 根据应用场景决定是否启用加上时间作为ClientId
    QTime current_time = QTime::currentTime();
    QString time = QString("%1:%2:%3").arg(current_time.hour(), 2)
                       .arg(current_time.minute(), 2)
                       .arg(current_time.second(), 2);
    _mqttClient->setClientId(_MqttConfig->deviceId()+"/APP/"+time);
    */
    _mqttClient->setClientId(_MqttConfig->deviceId()+"/APP");

    /**
     * 在setTopics() 接口中，自动设置 订阅与发布话题
     * 订阅话题: testtopic/MAV
     * 发布话题: testtopic/GCS
     * 代码会自动添加：
        /MAV后缀用于订阅（接收来自飞控的消息）
        /GCS后缀用于发布（发送消息到飞控）
     * 每个无人机有自己独立的消息通道，保持发布和订阅路径的对称性
     */
    _mqttClient->setTopics("testTop0512");    // 设置MQTT主题（topic）的基础前缀
    // 区分不同设备或应用：可以是一个客户ID、设备ID或应用标识符，确保不同设备/用户的消息不会互相干扰

    _mqttClient->setAutoReconnectInterval(60);
    _mqttClient->setAutoReconnect(true);

    /**
     * EMQX 访问控制 - 客户端认证 - 用户创建的用户名与密码
     * https://cloud.emqx.com
     */
    bool isNeedUserAndPs = false;  // 有些 Mqtt服务器不需要用户名与密码，根据服务器设置调整此处代码
    if(isNeedUserAndPs) {
        _mqttClient->setUsername("username");
        _mqttClient->setPassword("password");
    }

    _mqttClient->connectToHost();
    QObject::connect(_mqttClient, &QMQTTClient::messageReceived, this, &MqttLink::readBytes);

    _isConnected = true;

    emit connected();
    return true;
}

/*
void MqttLink::_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    //emit communicationError(tr("Link Error"), tr("Error on link %1. Error on socket: %2.").arg(getName()).arg(_socket->errorString()));
}
*/

/**
 * @brief Check if connection is active.
 *
 * @return True if link is connected, false otherwise.
 **/
bool MqttLink::isConnected() const
{
    if (!_isConnected)
        return false;

    return _mqttClient->isConnected();
}

MqttConfiguration::MqttConfiguration(const QString& name) : LinkConfiguration(name)
{
    _host = DEAFAULT_HOST;
    _deviceId = QString("");
}

MqttConfiguration::MqttConfiguration(MqttConfiguration* source) : LinkConfiguration(source)
{
    _host       = source->host();
    _deviceId   = source->deviceId();
}

void MqttConfiguration::copyFrom(LinkConfiguration *source)
{
    LinkConfiguration::copyFrom(source);
    auto* usource = dynamic_cast<MqttConfiguration*>(source);
    Q_ASSERT(usource != NULL);
    _host       = usource->host();
    _deviceId   = usource->deviceId();
}

void MqttConfiguration::setHost(const QString host)
{
    QString ipAdd = get_ip_address(host);
    if(ipAdd.isEmpty()) {
        qWarning() << "MQTT:" << "Could not resolve host:" << host;
    } else {
        _host = host;
    }
}

void MqttConfiguration::setDeviceId(QString deviceId)
{
    _deviceId = deviceId;
}

void MqttConfiguration::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("host", host());
    settings.setValue("deviceId", deviceId());
    settings.endGroup();
}

void MqttConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _host = settings.value("host", "host").toString();
    _deviceId = settings.value("deviceId","deviceId").toString();
    settings.endGroup();
}
