#include "ndi_sender.h"
#include "senderworker.h"
#include <QDebug>
#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <QStringList>
#include <wingdi.h>
#include <QDebug>
#include <sstream>

NDISender::NDISender(QObject *parent)
    : QObject(parent), ndiSend_(nullptr), workerThread(nullptr), senderWorker(nullptr)
{
}

NDISender::~NDISender()
{
    stopAll();  // Ensure all sending is stopped and resources are cleaned up
    if (initialized_) {
        NDIlib_send_destroy(ndiSend_);
        NDIlib_destroy();
        initialized_ = false;
    }
}

bool NDISender::initializeNDI()
{
    if (initialized_) {
        qDebug() << "NDI sender already initialized.";
        return true;
    }

    qDebug() << "Initializing NDI library...";
    if (!NDIlib_initialize()) {
        qDebug() << "Failed to initialize NDI library.";
        return false;
    }

    NDIlib_send_create_t sendCreateDesc = {0};
    sendCreateDesc.p_ndi_name = "NDI Test Sender";

    ndiSend_ = NDIlib_send_create(&sendCreateDesc);
    if (!ndiSend_) {
        qDebug() << "Failed to create NDI sender instance.";
        NDIlib_destroy();
        return false;
    }

    initialized_ = true;
    qDebug() << "NDI sender instance created successfully.";
    return true;
}

void NDISender::startScreenCapture()
{
    if (workerThread || !ndiSend_) return;

    workerThread = new QThread(this);
    senderWorker = new SenderWorker(ndiSend_);
    senderWorker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, senderWorker, &SenderWorker::startScreenCapture);
    connect(workerThread, &QThread::finished, senderWorker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
    qDebug() << "Started screen capture thread.";
}

void NDISender::startCameraFeed(const std::string &cameraID)
{
    if (workerThread || !ndiSend_) return;

    workerThread = new QThread(this);
    senderWorker = new SenderWorker(ndiSend_);
    senderWorker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, [this, cameraID]() { senderWorker->startCameraFeed(cameraID); });
    connect(workerThread, &QThread::finished, senderWorker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
    qDebug() << "Started camera feed thread.";
}

void NDISender::startAudioFeed(const std::string &audioSource)
{
    if (workerThread || !ndiSend_) return;

    workerThread = new QThread(this);
    senderWorker = new SenderWorker(ndiSend_);
    senderWorker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, [this, audioSource]() { senderWorker->startAudioFeed(audioSource); });
    connect(workerThread, &QThread::finished, senderWorker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
    qDebug() << "Started audio feed thread.";
}

void NDISender::sendMessage(const std::string &message, int priority, const std::string &game)
{
    if (!senderWorker) {
        qDebug() << "SenderWorker not initialized.";
        return;
    }
    senderWorker->sendMessage(message, priority, game);
}

void NDISender::stopAll()
{
    if (senderWorker) {
        senderWorker->stopAll();
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            workerThread = nullptr;
            senderWorker = nullptr;
        }
        qDebug() << "Stopped all sending tasks.";
    }
}
