#include "ndi_sender.h"
#include <QDebug>
#include <sstream> // Include this header

NDISender::NDISender(QObject *parent)
    : QObject(parent),
    initialized_(false),
    ndiSendCameraMic_(nullptr),
    ndiSendScreenShare_(nullptr),
    workerThread_(nullptr),
    senderWorker_(nullptr)
{
}

NDISender::~NDISender()
{
    stopAll();
    if (initialized_) {
        if (ndiSendCameraMic_) {
            NDIlib_send_destroy(ndiSendCameraMic_);
            ndiSendCameraMic_ = nullptr;
        }
        if (ndiSendScreenShare_) {
            NDIlib_send_destroy(ndiSendScreenShare_);
            ndiSendScreenShare_ = nullptr;
        }
        NDIlib_destroy();
        initialized_ = false;
    }
}


bool NDISender::initializeNDI()
{
    if (initialized_) {
        qDebug() << "NDI Sender already initialized.";
        return true;
    }

    qDebug() << "Initializing NDI Sender...";
    if (!NDIlib_initialize()) {
        qWarning() << "Failed to initialize NDI library.";
        return false;
    }

    // Create NDI send instance for camera and mic
    NDIlib_send_create_t sendDescCameraMic = {};
    sendDescCameraMic.p_ndi_name = "Camera and Mic Stream";
    ndiSendCameraMic_ = NDIlib_send_create(&sendDescCameraMic);
    if (!ndiSendCameraMic_) {
        qWarning() << "Failed to create NDI send instance for camera and mic.";
        NDIlib_destroy();
        return false;
    }

    // Create NDI send instance for screen share
    NDIlib_send_create_t sendDescScreenShare = {};
    sendDescScreenShare.p_ndi_name = "Screen Share Stream";
    ndiSendScreenShare_ = NDIlib_send_create(&sendDescScreenShare);
    if (!ndiSendScreenShare_) {
        qWarning() << "Failed to create NDI send instance for screen share.";
        NDIlib_send_destroy(ndiSendCameraMic_);
        NDIlib_destroy();
        return false;
    }

    initialized_ = true;
    qDebug() << "NDI Sender initialized successfully.";
    return true;
}

void NDISender::startAllStreams(const QString &cameraID, const QString &audioSource, const QString &applicationName)
{
    if (!initialized_) {
        qWarning() << "NDI Sender not initialized.";
        return;
    }

    if (senderWorker_) {
        qWarning() << "Sender already running.";
        return;
    }

    qDebug() << "Starting all streams with camera ID:" << cameraID << ", audio source:" << audioSource << ", application:" << applicationName;

    senderWorker_ = new SenderWorker(ndiSendCameraMic_, ndiSendScreenShare_);
    workerThread_ = new QThread(this);
    senderWorker_->moveToThread(workerThread_);

    connect(workerThread_, &QThread::started, senderWorker_, [=]() {
        senderWorker_->start(cameraID, audioSource, applicationName);
    });

    connect(senderWorker_, &SenderWorker::finished, workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished, senderWorker_, &QObject::deleteLater);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);

    workerThread_->start();
}

void NDISender::sendMessage(const QString &message, int priority, const QString &application)
{
    if (!ndiSendCameraMic_) {
        qWarning() << "NDI send instance not initialized.";
        return;
    }

    std::stringstream jsonMessage;
    jsonMessage << "{\"priority\":" << priority
                << ",\"message\":\"" << message.toStdString()
                << "\",\"application\":\"" << application.toStdString() << "\"}";

    std::string jsonString = jsonMessage.str();

    NDIlib_metadata_frame_t metadataFrame;
    metadataFrame.p_data = const_cast<char *>(jsonString.c_str());
    metadataFrame.length = static_cast<int>(jsonString.length());
    metadataFrame.timecode = NDIlib_send_timecode_synthesize;

    NDIlib_send_add_connection_metadata(ndiSendCameraMic_, &metadataFrame);
}

void NDISender::stopAll()
{
    if (senderWorker_) {
        senderWorker_->stopAll();
        if (workerThread_) {
            workerThread_->quit();
            workerThread_->wait();
            delete workerThread_;
            workerThread_ = nullptr;
            senderWorker_ = nullptr;
        }
        qDebug() << "All sending tasks stopped.";
    }
}
