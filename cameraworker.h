#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QImage>

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(const QString &cameraID, QObject *parent = nullptr);
    ~CameraWorker();

signals:
    void frameReady(const QImage &frame);

public slots:
    void startCamera();
    void stopCamera();

private slots:
    void onVideoFrameChanged(const QVideoFrame &frame);

private:
    QString cameraID_;
    QCamera *camera_;
    QMediaCaptureSession *captureSession_;
    QVideoSink *videoSink_;
};

#endif // CAMERAWORKER_H
