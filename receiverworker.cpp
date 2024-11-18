#include "receiverworker.h"
#include <QThread>
#include <QDebug>

ReceiverWorker::ReceiverWorker(NDIlib_recv_instance_t ndiReceiverInstance, QObject *parent)
    : QObject(parent), ndiReceiverInstance(ndiReceiverInstance), receiving(true)
{
}

ReceiverWorker::~ReceiverWorker()
{
}

void ReceiverWorker::process()
{
    qDebug() << "ReceiverWorker::process() started.";
    while (receiving) {
        NDIlib_video_frame_v2_t videoFrame = {};
        auto frameType = NDIlib_recv_capture_v2(ndiReceiverInstance, &videoFrame, nullptr, nullptr, 1000);

        if (!receiving) {
            break;
        }

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
        case NDIlib_frame_type_none:
            // No data received within timeout
            break;
        default:
            qDebug() << "Error receiving data. Frame type:" << frameType;
            break;
        }

        QThread::msleep(10);
    }
    qDebug() << "ReceiverWorker::process() finished.";
}

void ReceiverWorker::stop()
{
    receiving = false;
}
