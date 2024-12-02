#include "ndi_receiver.h"
#include "discoveryworker.h"
#include "receiverworker.h"
#include <iostream>
#include <QDebug>

NDIReceiver::NDIReceiver(QObject *parent)
    : QObject(parent), ndiFindInstance(nullptr),
    discoveryThread(nullptr), discoveryWorker(nullptr)
{
}

NDIReceiver::~NDIReceiver()
{
    terminateReceiver();
}

bool NDIReceiver::initializeReceiver()
{
    if (!NDIlib_initialize()) {
        std::cerr << "Failed to initialize NDI." << std::endl;
        return false;
    }

    ndiFindInstance = NDIlib_find_create_v2();
    if (!ndiFindInstance) {
        std::cerr << "Failed to create NDI finder." << std::endl;
        return false;
    }

    qDebug() << "NDI Receiver initialized successfully.";
    return true;
}

void NDIReceiver::startDiscovery()
{
    if (discoveryThread) {
        return;
    }

    discoveryThread = new QThread(this);
    discoveryWorker = new DiscoveryWorker(ndiFindInstance);

    discoveryWorker->moveToThread(discoveryThread);

    connect(discoveryThread, &QThread::started, discoveryWorker, &DiscoveryWorker::process);
    connect(discoveryWorker, &DiscoveryWorker::sourcesDiscovered, this, &NDIReceiver::sourcesDiscovered);
    connect(discoveryWorker, &DiscoveryWorker::sourcesDiscovered, discoveryThread, &QThread::quit);
    connect(discoveryThread, &QThread::finished, discoveryWorker, &QObject::deleteLater);
    connect(discoveryThread, &QThread::finished, discoveryThread, &QObject::deleteLater);
    connect(discoveryThread, &QThread::finished, [this]() {
        discoveryThread = nullptr;
        discoveryWorker = nullptr;
    });

    discoveryThread->start();
}

void NDIReceiver::selectSource(const QString &sourceName)
{
    std::lock_guard<std::mutex> lock(sourcesMutex);

    if (ndiReceiverInstances.find(sourceName) != ndiReceiverInstances.end()) {
        qDebug() << "Already receiving from source:" << sourceName;
        return;
    }

    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(ndiFindInstance, &numSources);

    for (uint32_t i = 0; i < numSources; ++i) {
        if (QString::fromUtf8(sources[i].p_ndi_name) == sourceName) {
            NDIlib_recv_create_v3_t recv_create_desc = {0};
            recv_create_desc.source_to_connect_to = sources[i];
            recv_create_desc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
            recv_create_desc.bandwidth = NDIlib_recv_bandwidth_highest;
            recv_create_desc.allow_video_fields = false;

            NDIlib_recv_instance_t ndiReceiverInstance = NDIlib_recv_create_v3(&recv_create_desc);

            if (!ndiReceiverInstance) {
                qDebug() << "Failed to create NDI receiver instance for source:" << sourceName;
                return;
            }

            ndiReceiverInstances[sourceName] = ndiReceiverInstance;

            QThread *thread = new QThread(this);
            ReceiverWorker *worker = new ReceiverWorker(ndiReceiverInstance);

            worker->moveToThread(thread);

            connect(thread, &QThread::started, worker, &ReceiverWorker::start);
            connect(worker, &ReceiverWorker::frameReceived, this, [this, sourceName](const QImage &frame) {
                emit frameReceived(sourceName, frame);
            });
            connect(this, &NDIReceiver::stopReceivingSignal, worker, &ReceiverWorker::stop, Qt::QueuedConnection);

            // Add this connection
            connect(worker, &ReceiverWorker::finished, thread, &QThread::quit);

            connect(thread, &QThread::finished, worker, &QObject::deleteLater);
            connect(thread, &QThread::finished, thread, &QObject::deleteLater);

            receiverThreads[sourceName] = thread;
            receiverWorkers[sourceName] = worker;

            thread->start();

            qDebug() << "Started receiving from source:" << sourceName;
        }
    }
}

void NDIReceiver::stopReceiving()
{
    qDebug() << "NDIReceiver::stopReceiving() called. Emitting stopReceivingSignal.";
    emit stopReceivingSignal();

    // Wait for all threads to finish
    for (auto &pair : receiverThreads) {
        QThread *thread = pair.second;
        if (thread->isRunning()) {
            qDebug() << "Waiting for thread to finish...";
            thread->wait(20);
            qDebug() << "Thread finished.";
        }
    }

    // Clean up threads and workers
    for (auto &pair : receiverThreads) {
        delete pair.second; // Delete QThread
    }
    receiverThreads.clear();
    receiverWorkers.clear();
    ndiReceiverInstances.clear();
}



void NDIReceiver::terminateReceiver()
{
    stopReceiving();

    if (discoveryThread) {
        discoveryThread->quit();
        discoveryThread->wait();
        delete discoveryThread;
        discoveryThread = nullptr;
        discoveryWorker = nullptr;
    }

    if (ndiFindInstance) {
        NDIlib_find_destroy(ndiFindInstance);
        ndiFindInstance = nullptr;
    }

    NDIlib_destroy();
}
