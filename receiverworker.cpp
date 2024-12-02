#include "receiverworker.h"
#include <QDebug>

ReceiverWorker::ReceiverWorker(NDIlib_recv_instance_t ndiReceiverInstance, QObject *parent)
    : QObject(parent),
    ndiReceiverInstance(ndiReceiverInstance),
    captureTimer(new QTimer(this))
{
    connect(captureTimer, &QTimer::timeout, this, &ReceiverWorker::captureFrame);
}

ReceiverWorker::~ReceiverWorker()
{
    qDebug() << "Destroyed Receiver Worker";
    if (ndiReceiverInstance) {
        NDIlib_recv_destroy(ndiReceiverInstance);
        ndiReceiverInstance = nullptr;
    }
}

void ReceiverWorker::start()
{
    captureTimer->start(10); // Adjust interval as needed
}

void ReceiverWorker::stop()
{
    qDebug() << "ReceiverWorker::stop() called.";
    if (captureTimer->isActive()) {
        captureTimer->stop();
    }
    if (ndiReceiverInstance) {
        NDIlib_recv_connect(ndiReceiverInstance,NULL);
    }

    emit finished();
    qDebug() <<"Finished Emitted";
}

void ReceiverWorker::captureFrame()
{
    NDIlib_video_frame_v2_t videoFrame = {};

    // Capture video frames with a timeout of 0 (non-blocking)
    auto frameType = NDIlib_recv_capture_v2(ndiReceiverInstance, &videoFrame, nullptr, nullptr, 0);

    switch (frameType) {
    case NDIlib_frame_type_video:
    {
        if (videoFrame.p_data == nullptr) {
            qDebug() << "Received video frame with null data.";
            break;
        }

        QImage image(
            (uchar*)videoFrame.p_data,
            videoFrame.xres,
            videoFrame.yres,
            videoFrame.line_stride_in_bytes,
            QImage::Format_ARGB32
            );

        if (!image.isNull()) {
            emit frameReceived(image.copy());
        }

        NDIlib_recv_free_video_v2(ndiReceiverInstance, &videoFrame);
        break;
    }
    case NDIlib_frame_type_status_change:
        qDebug() << "ReceiverWorker: Source connection status changed.";
        break;
    case NDIlib_frame_type_none:
        // No data received
        break;
    default:
        qDebug() << "Error receiving data. Frame type:" << frameType;
        break;
    }
}
