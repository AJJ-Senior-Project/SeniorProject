#include "cameraworker.h"
#include <QMediaDevices>
#include <QDebug>
#include <QThread>

CameraWorker::CameraWorker(const QString &cameraID, QObject *parent)
    : QObject(parent),
    cameraID_(cameraID),
    camera_(nullptr),
    captureSession_(nullptr),
    videoSink_(nullptr)
{
}

CameraWorker::~CameraWorker()
{
    stopCamera();
}

void CameraWorker::startCamera()
{
    qDebug() << "CameraWorker: Starting camera with ID:" << cameraID_;

    // Find the camera by ID
    QCameraDevice selectedCamera;
    auto cameras = QMediaDevices::videoInputs();
    for (const auto &cam : cameras) {
        if (cam.id() == cameraID_) {
            selectedCamera = cam;
            break;
        }
    }

    if (selectedCamera.isNull()) {
        qWarning() << "CameraWorker: Selected camera not found.";
        return;
    }

    camera_ = new QCamera(selectedCamera);
    captureSession_ = new QMediaCaptureSession();
    videoSink_ = new QVideoSink();

    captureSession_->setCamera(camera_);
    captureSession_->setVideoSink(videoSink_);

    connect(videoSink_, &QVideoSink::videoFrameChanged, this, &CameraWorker::onVideoFrameChanged);

    camera_->start();
    qDebug() << "CameraWorker: Camera started.";
}

void CameraWorker::stopCamera()
{
    if (camera_) {
        camera_->stop();
        delete captureSession_;
        captureSession_ = nullptr;
        delete camera_;
        camera_ = nullptr;
        delete videoSink_;
        videoSink_ = nullptr;
        qDebug() << "CameraWorker: Camera stopped.";
    }
}

void CameraWorker::onVideoFrameChanged(const QVideoFrame &frame)
{
    QVideoFrame videoFrame = frame;

    if (!videoFrame.isValid()) {
        qWarning() << "CameraWorker: Invalid video frame received.";
        return;
    }

    if (!videoFrame.map(QVideoFrame::ReadOnly)) {
        qWarning() << "CameraWorker: Failed to map video frame.";
        return;
    }

    QImage::Format imageFormat = QVideoFrameFormat::imageFormatFromPixelFormat(videoFrame.pixelFormat());
    const uchar *bits = videoFrame.bits(0);
    int bytesPerLine = videoFrame.bytesPerLine(0);

    QImage image(bits, videoFrame.width(), videoFrame.height(), bytesPerLine, imageFormat);

    videoFrame.unmap();

    if (image.isNull()) {
        qWarning() << "CameraWorker: Received invalid image from camera.";
        return;
    }

    // Emit the frame to be processed elsewhere
    emit frameReady(image.copy());  // Copy the image to ensure thread safety
}
