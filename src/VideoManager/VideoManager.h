﻿/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef VideoManager_H
#define VideoManager_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QUrl>

#include "QGCMAVLink.h"
#include "QGCLoggingCategory.h"
#include "VideoReceiver.h"
#include "QGCToolbox.h"
#include "SubtitleWriter.h"

Q_DECLARE_LOGGING_CATEGORY(VideoManagerLog)

class VideoSettings;
class Vehicle;
class Joystick;

class VideoManager : public QGCTool
{
    Q_OBJECT

public:
    VideoManager    (QGCApplication* app, QGCToolbox* toolbox);
    virtual ~VideoManager   ();

    Q_PROPERTY(bool             hasVideo                READ    hasVideo                                    NOTIFY hasVideoChanged)
    Q_PROPERTY(bool             isGStreamer             READ    isGStreamer                                 NOTIFY isGStreamerChanged)
    Q_PROPERTY(bool             isUvc                   READ    isUvc                                       NOTIFY isUvcChanged)
    Q_PROPERTY(bool             isTaisync               READ    isTaisync       WRITE   setIsTaisync        NOTIFY isTaisyncChanged)
    Q_PROPERTY(QString          uvcVideoSourceID        READ    uvcVideoSourceID                            NOTIFY uvcVideoSourceIDChanged)
    Q_PROPERTY(bool             uvcEnabled              READ    uvcEnabled                                  CONSTANT)
    Q_PROPERTY(bool             fullScreen              READ    fullScreen      WRITE   setfullScreen       NOTIFY fullScreenChanged)
    // 第一路图传
    Q_PROPERTY(VideoReceiver*   videoReceiver           READ    videoReceiver                               CONSTANT)
    // 第二路图传
    Q_PROPERTY(VideoReceiver*   thermalVideoReceiver    READ    thermalVideoReceiver                        CONSTANT)
    // 第三路图传
    Q_PROPERTY(VideoReceiver*   thirdVideoReceiver      READ    thirdVideoReceiver                          CONSTANT)
    // 第四路图传
    Q_PROPERTY(VideoReceiver*   fourthVideoReceiver     READ    fourthVideoReceiver                          CONSTANT)

    Q_PROPERTY(VideoReceiver*   thermalVideoReceiver    READ    thermalVideoReceiver                        CONSTANT)
    Q_PROPERTY(double           aspectRatio             READ    aspectRatio                                 NOTIFY aspectRatioChanged)
    Q_PROPERTY(double           thermalAspectRatio      READ    thermalAspectRatio                          NOTIFY aspectRatioChanged)
    Q_PROPERTY(double           hfov                    READ    hfov                                        NOTIFY aspectRatioChanged)
    Q_PROPERTY(double           thermalHfov             READ    thermalHfov                                 NOTIFY aspectRatioChanged)
    Q_PROPERTY(bool             autoStreamConfigured    READ    autoStreamConfigured                        NOTIFY autoStreamConfiguredChanged)
    Q_PROPERTY(bool             hasThermal              READ    hasThermal                                  NOTIFY decodingChanged)
    Q_PROPERTY(QString          imageFile               READ    imageFile                                   NOTIFY imageFileChanged)
    Q_PROPERTY(bool             streaming               READ    streaming                                   NOTIFY streamingChanged)
    Q_PROPERTY(bool             decoding                READ    decoding                                    NOTIFY decodingChanged)
    Q_PROPERTY(bool             recording               READ    recording                                   NOTIFY recordingChanged)
    Q_PROPERTY(QSize            videoSize               READ    videoSize                                   NOTIFY videoSizeChanged)

    virtual bool        hasVideo            ();
    virtual bool        isGStreamer         ();
    virtual bool        isUvc               ();
    virtual bool        isTaisync           () { return _isTaisync; }
    virtual bool        fullScreen          () { return _fullScreen; }
    virtual QString     uvcVideoSourceID    () { return _uvcVideoSourceID; }
    virtual double      aspectRatio         ();
    virtual double      thermalAspectRatio  ();
    virtual double      hfov                ();
    virtual double      thermalHfov         ();
    virtual bool        autoStreamConfigured();
    virtual bool        hasThermal          ();
    virtual QString     imageFile           ();

    bool streaming(void) {
        return _streaming;
    }

    bool decoding(void) {
        return _decoding;
    }

    bool recording(void) {
        return _recording;
    }

    QSize videoSize(void) {
        const quint32 size = _videoSize;
        return QSize((size >> 16) & 0xFFFF, size & 0xFFFF);
    }

// FIXME: AV: they should be removed after finishing multiple video stream support
// new arcitecture does not assume direct access to video receiver from QML side, even if it works for now
    virtual VideoReceiver*  videoReceiver           () { return _videoReceiver[0]; }
    virtual VideoReceiver*  thermalVideoReceiver    () { return _videoReceiver[1]; }
    virtual VideoReceiver*  thirdVideoReceiver      () { return _videoReceiver[2]; }
    virtual VideoReceiver*  fourthVideoReceiver     () { return _videoReceiver[3]; }

#if defined(QGC_DISABLE_UVC)
    virtual bool        uvcEnabled          () { return false; }
#else
    virtual bool        uvcEnabled          ();
#endif

    virtual void        setfullScreen       (bool f);
    virtual void        setIsTaisync        (bool t) { _isTaisync = t;  emit isTaisyncChanged(); }

    // Override from QGCTool
    virtual void        setToolbox          (QGCToolbox *toolbox);

    Q_INVOKABLE void startVideo     ();
    Q_INVOKABLE void stopVideo      ();

    Q_INVOKABLE void startRecording (const QString& videoFile = QString());
    Q_INVOKABLE void stopRecording  ();

    Q_INVOKABLE void grabImage(const QString& imageFile = QString());

    /* 新增多路图传信号槽绑定  */
    // 绑定各通道的启动/停止信号槽（多路共用）
    void _bindSecondaryStream(VideoReceiver* receiver, int index);

signals:
    void hasVideoChanged            ();
    void isGStreamerChanged         ();
    void isUvcChanged               ();
    void uvcVideoSourceIDChanged    ();
    void fullScreenChanged          ();
    void isAutoStreamChanged        ();
    void isTaisyncChanged           ();
    void aspectRatioChanged         ();
    void autoStreamConfiguredChanged();
    void imageFileChanged           ();
    void streamingChanged           ();
    void decodingChanged            ();
    void recordingChanged           ();
    void recordingStarted           ();
    void videoSizeChanged           ();

protected slots:
    void _videoSourceChanged        ();
    void _udpPortChanged            ();
    // 新增多路图传模块
    void _rtspUrlChanged            ();
    void _rtspUrl02Changed          ();
    void _rtspUrl03Changed          ();
    void _rtspUrl04Changed          ();
    void _tcpUrlChanged             ();
    void _lowLatencyModeChanged     ();
    void _updateUVC                 ();
    void _setActiveVehicle          (Vehicle* vehicle);
    void _aspectRatioChanged        ();
    void _communicationLostChanged  (bool communicationLost);

protected:
    friend class FinishVideoInitialization;

    void _initVideo                 ();
    bool _updateSettings            (unsigned id);
    bool _updateVideoUri            (unsigned id, const QString& uri);
    void _cleanupOldVideos          ();
    void _restartAllVideos          ();
    void _restartVideo              (unsigned id);
    void _startReceiver             (unsigned id);
    void _stopReceiver              (unsigned id);

protected:

    // ***************** 图传数量 *****************
    // 成员变量：支持4路图传
    int videoCounts = 4;

    QString                 _videoFile;
    QString                 _imageFile;
    SubtitleWriter          _subtitleWriter;
    bool                    _isTaisync              = false;
    VideoReceiver*          _videoReceiver[4]       = { nullptr, nullptr, nullptr, nullptr }; // 新增4路图传
    void*                   _videoSink[4]           = { nullptr, nullptr, nullptr, nullptr }; // 新增4路图传
    QString                 _videoUri[4]; // 新增4路图传
    // FIXME: AV: _videoStarted seems to be access from 3 different threads, from time to time
    // 1) Video Receiver thread
    // 2) Video Manager/main app thread
    // 3) Qt rendering thread (during video sink creation process which should happen in this thread)
    // It works for now but...
    bool                    _videoStarted[4]        = { false, false, false, false }; // 新增4路图传
    bool                    _lowLatencyStreaming[4] = { false, false, false, false }; // 新增4路图传
    QAtomicInteger<bool>    _streaming              = false;
    QAtomicInteger<bool>    _decoding               = false;
    QAtomicInteger<bool>    _recording              = false;
    QAtomicInteger<quint32> _videoSize              = 0;
    VideoSettings*          _videoSettings          = nullptr;
    QString                 _uvcVideoSourceID;
    bool                    _fullScreen             = false;
    Vehicle*                _activeVehicle          = nullptr;
};

#endif
