#ifndef QMQTTCLIENT_H
#define QMQTTCLIENT_H

#include <QWidget>
#include <QTimer>
#include <QHostAddress>
#include <QDate>
#include "qmqtt.h"

class QMQTTClient : public QMQTT::Client
{
    Q_OBJECT
public:
    QMQTTClient(const QHostAddress& host = QHostAddress::LocalHost,
                       const quint16 port = 1883,
                        QString topic = "test",
                        QObject* parent = NULL,
                        bool encrypt = false)
        : QMQTT::Client(host, port, parent)
        , _number(0){

        //qDebug() << "[����MQTT] topic:" << topic;

        encrypt_ = encrypt;

        //this->setClientId("any"); // ����ʱ����
        //this->setTopics(topic); // ����ʱ���û���

        this->setKeepAlive(60);
        this->setCleanSession(1);

        connect(this, &QMQTTClient::connected, this, &QMQTTClient::onConnected);
        connect(this, &QMQTTClient::disconnected, this, &QMQTTClient::onDisconnected);
        connect(this, &QMQTTClient::subscribed, this, &QMQTTClient::onSubscribed);
        connect(this, &QMQTTClient::received, this, &QMQTTClient::onReceived);
    }

    QMQTTClient(const QString& hostName ,
                       const quint16 port ,
                        const QSslConfiguration& sslConfig,
                        QObject* parent = NULL,
                        bool encrypt = false)
        : QMQTT::Client(hostName, port, sslConfig)
        , _number(0){

        qDebug() << "[����Mqtt��ʽ2]";

        encrypt_ = encrypt;
        _isConnected =false;
        this->setClientId("any");
        this->setKeepAlive(60);
        this->setCleanSession(1);

        connect(this, &QMQTTClient::connected, this, &QMQTTClient::onConnected);
        connect(this, &QMQTTClient::disconnected, this, &QMQTTClient::onDisconnected);
        connect(this, &QMQTTClient::subscribed, this, &QMQTTClient::onSubscribed);
        connect(this, &QMQTTClient::received, this, &QMQTTClient::onReceived);
    }

    virtual ~QMQTTClient() {}

    void setTopicSub(QString topic = "testtopic/MAV"){
        _topicSub = topic;
    }
    void setTopicPub(QString topic = "testtopic/GCS"){
        _topicPub = topic;
    }

    void setTopics(QString topic){
        _topic = topic;
        QString topicSub = topic + "/MAV";  // ���Ļ��⣨���ķɿط��͵����ݣ�
        QString topicPub = topic + "/GCS";  // �������⣨���͵���վ���ݵ��ɿضˣ�

       // qDebug() << "[MQTT] ���û��� " << "���ķɿػ���:" << topicSub << " ��������վ����:" << topicPub;
        qDebug() << "[MQTT] Set Topic " << "Sub:" << topicSub << " Pub:" << topicPub;

        setTopicSub(topicSub);
        setTopicPub(topicPub);
    }

    void publishOne(QString msg){
        QString sendmsg = msg;

        if(encrypt_)
            sendmsg = encrypt(msg);

        QMQTT::Message message(_number, _topicPub, sendmsg.toUtf8());
        publish(message);
        _number++;
    }

    void publishMsg(QByteArray msg){
        QMQTT::Message message(_number, _topicPub, msg);
        //message.setQos(2);
        publish(message);
        _number++;
    }

    bool encrypt_;
    bool _isConnected;
    quint16 _number;
    QString _topic;

    QString _topicPub;
    QString _topicSub;

    QString encrypt(QString msg){
        // TODO
        return msg;
    }

    bool isConnected(){
        return _isConnected;
    }

public slots:
    void onConnected(){
        //qDebug() << "[����MQTT] " << _topic;
        subscribe(_topicSub, 0);
        _isConnected =true;
        qDebug() << "[MQTT] Connected" << _topic << QDateTime::currentDateTime().toString("hh:mm:ss");
    }

    void onDisconnected(){
        //qDebug() << "[�Ͽ�MQTT] " << _topic;
        _isConnected =false;
        //qDebug()<< "[MQTT] �ѶϿ� " << _topic << QDateTime::currentDateTime().toString("hh:mm:ss");
    }

    void onSubscribed(const QString& topic){
        emit subscribedTo(topic);
    }

    void onReceived(const QMQTT::Message& message){
        QString msg = QString::fromUtf8(message.payload());

        if(encrypt_)
            msg = encrypt(msg);

        emit messageReceived(message.payload());
    }

signals:
    void messageReceived(QByteArray msg);
    void subscribedTo(QString topic);
};

#endif // QMQTTCLIENT_H
