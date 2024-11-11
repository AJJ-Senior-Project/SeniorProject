#include "receiverworker.h"
#include <iostream>
#include <QThread>
#include <qdebug.h>

ReceiverWorker::ReceiverWorker(NDIlib_recv_instance_t ndiReceiverInstance, QObject *parent)
    : QObject(parent), ndiReceiverInstance(ndiReceiverInstance), receiving(true)
{
    qDebug() << "ReceiverWorker created. Receiving flag is set to" << receiving.load();
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
            // Exit loop if receiving has been set to false
            break;
        }

        switch (frameType) {
        case NDIlib_frame_type_video:
        {
            qDebug() << "Received a video frame.";
            if (videoFrame.p_data == nullptr) {
                std::cerr << "videoFrame.p_data is null!" << std::endl;
                break;
            }

            QImage image(
                (uchar*)videoFrame.p_data,
                videoFrame.xres,
                videoFrame.yres,
                videoFrame.line_stride_in_bytes,
                QImage::Format_ARGB32
                );

            if (image.isNull()) {
                std::cerr << "Failed to create QImage." << std::endl;
                break;
            }

            std::cout << "Received video frame: xres=" << videoFrame.xres
                      << ", yres=" << videoFrame.yres
                      << ", FourCC=" << videoFrame.FourCC
                      << ", line_stride_in_bytes=" << videoFrame.line_stride_in_bytes << std::endl;

            emit frameReceived(image.copy());

            NDIlib_recv_free_video_v2(ndiReceiverInstance, &videoFrame);
            break;
        }
        case NDIlib_frame_type_none:
            // No data received within timeout
            break;
        default:
            std::cerr << "Error receiving data. Frame type: " << frameType << std::endl;
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

