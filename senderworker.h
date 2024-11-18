#ifndef SENDERWORKER_H
#define SENDERWORKER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <atomic>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMutex>
#include <QThread>
#include <QMediaDevices>
#include <QTimer>

class SenderWorker : public QObject
{
    Q_OBJECT
public:
    explicit SenderWorker(NDIlib_send_instance_t ndiSendCameraMicInstance,
                          NDIlib_send_instance_t ndiSendScreenShareInstance,
                          QObject *parent = nullptr);
    ~SenderWorker();

public slots:
    void start(const QString &cameraID, const QString &audioSource, const QString &applicationName);
    void stopAll();

signals:
    void finished();

private slots:
    void onCameraFrameReceived(const QVideoFrame &frame);
    void captureScreen();
    void captureAudioFeed();

private:
    void startCameraFeed(const QString &cameraID);
    void startAudioFeed(const QString &audioSource);
    void startScreenCapture(const QString &applicationName);

    NDIlib_send_instance_t ndiSendCameraMicInstance_;
    NDIlib_send_instance_t ndiSendScreenShareInstance_;
    HWND targetWindowHandle_;

    bool sending_;
    QTimer *screenCaptureTimer_;
    QTimer *audioCaptureTimer_;

    QMutex audioMutex_;
    QByteArray audioBuffer_;

    QCamera *camera_;
    QMediaCaptureSession *captureSession_;
    QVideoSink *videoSink_;

    QAudioSource *audioInput_;
    QIODevice *audioIO_;
};

#endif // SENDERWORKER_H
