#include "ndi_sender.h"
#include <QDebug>

NDISender::NDISender()
{
    if (!NDIlib_initialize())
    {
        qDebug() << "Failure";
    }
}
NDISender::~NDISender()
{
    if (ndi_send_instance)
    {
        NDIlib_send_destroy(ndi_send_instance);
    }
    NDIlib_destroy();
}
// Startup Occurs With this
void NDISender::initializeSender()
{
    NDIlib_send_create_t NDI_send_create_desc;
    NDI_send_create_desc.p_ndi_name = "NDI Sender";
    ndi_send_instance = NDIlib_send_create(&NDI_send_create_desc);
    if (!ndi_send_instance)
    {
        qDebug() << "Failed at Initalize";
    }
}