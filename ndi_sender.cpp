#include "ndi_sender.h"
#include <QDebug>
#include <QImage>  // Make sure QImage is included

NDISender::NDISender() {
    // Initialize NDI
    if (!NDIlib_initialize()) {
        qDebug() << "Failed to initialize NDI";
    }
}

NDISender::~NDISender() {
    if (ndi_send_instance) {
        NDIlib_send_destroy(ndi_send_instance);
    }
    NDIlib_destroy();
}

void NDISender::initializeSender() {
    // Create NDI sender
    NDIlib_send_create_t NDI_send_create_desc;
    NDI_send_create_desc.p_ndi_name = "NDI Sender";
    ndi_send_instance = NDIlib_send_create(&NDI_send_create_desc);

    if (!ndi_send_instance) {
        qDebug() << "Failed to create NDI sender";
    }
}

void NDISender::sendFrame(const QImage &frame) {  // Definition of sendFrame
    // NDI frame sending logic
    NDIlib_video_frame_v2_t NDI_video_frame;
    NDI_video_frame.xres = frame.width();
    NDI_video_frame.yres = frame.height();
    // Set other frame properties as needed

    NDIlib_send_send_video_v2(ndi_send_instance, &NDI_video_frame);
}
