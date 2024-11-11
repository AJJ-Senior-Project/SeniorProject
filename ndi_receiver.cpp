#include "ndi_receiver.h"
#include "discoveryworker.h"
#include "receiverworker.h"
#include <iostream>
#include <qdebug.h>

NDIReceiver::NDIReceiver(QObject *parent)
    : QObject(parent), ndiFindInstance(nullptr), ndiReceiverInstance(nullptr),
    discoveryThread(nullptr), receiverThread(nullptr),
    discoveryWorker(nullptr), receiverWorker(nullptr)
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

    // Retrieve the current list of sources
    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(ndiFindInstance, &numSources);

    qDebug() << "Number of sources available:" << numSources;

    for (uint32_t i = 0; i < numSources; ++i) {
        qDebug() << "Available source:" << QString::fromUtf8(sources[i].p_ndi_name);
        if (QString::fromUtf8(sources[i].p_ndi_name) == sourceName) {
            if (ndiReceiverInstance) {
                NDIlib_recv_destroy(ndiReceiverInstance);
                ndiReceiverInstance = nullptr;
            }

            // Create the receiver instance with desired settings
            NDIlib_recv_create_v3_t recv_create_desc = {0};
            recv_create_desc.source_to_connect_to = sources[i];
            recv_create_desc.color_format = NDIlib_recv_color_format_BGRX_BGRA;
            recv_create_desc.bandwidth = NDIlib_recv_bandwidth_highest;
            recv_create_desc.allow_video_fields = false;

            ndiReceiverInstance = NDIlib_recv_create_v3(&recv_create_desc);

            if (!ndiReceiverInstance) {
                qDebug() << "Failed to create NDI receiver instance.";
                return;
            }

            qDebug() << "NDI Receiver Instance created successfully.";

            break;
        }
    }

    if (!ndiReceiverInstance) {
        qDebug() << "No matching source found for" << sourceName;
    }
}


void NDIReceiver::startReceiving()
{
    if (receiverThread || !ndiReceiverInstance) {
        qDebug() << "Receiver thread already running or ndiReceiverInstance is null.";
        if (receiverThread) qDebug() << "Receiver thread is already running.";
        if (!ndiReceiverInstance) qDebug() << "ndiReceiverInstance is null.";
        return;
    }

    receiverThread = new QThread(this);
    receiverWorker = new ReceiverWorker(ndiReceiverInstance);

    receiverWorker->moveToThread(receiverThread);

    connect(receiverThread, &QThread::started, receiverWorker, &ReceiverWorker::process);
    connect(receiverWorker, &ReceiverWorker::frameReceived, this, &NDIReceiver::frameReceived);
    connect(this, &NDIReceiver::stopReceivingSignal, receiverWorker, &ReceiverWorker::stop);
    connect(receiverThread, &QThread::finished, receiverWorker, &QObject::deleteLater);
    connect(receiverThread, &QThread::finished, receiverThread, &QObject::deleteLater);
    connect(receiverThread, &QThread::finished, [this]() {
        receiverThread = nullptr;
        receiverWorker = nullptr;
    });

    receiverThread->start();

    qDebug() << "Receiver thread started.";
}


void NDIReceiver::stopReceiving()
{
    if (receiverThread) {
        emit stopReceivingSignal();
        receiverThread->quit();
        // Do not wait here
    }

    if (ndiReceiverInstance) {
        NDIlib_recv_destroy(ndiReceiverInstance);
        ndiReceiverInstance = nullptr;
    }
}

void NDIReceiver::terminateReceiver()
{
    stopReceiving();

    if (ndiReceiverInstance) {
        NDIlib_recv_destroy(ndiReceiverInstance);
        ndiReceiverInstance = nullptr;
    }

    if (ndiFindInstance) {
        NDIlib_find_destroy(ndiFindInstance);
        ndiFindInstance = nullptr;
    }

    NDIlib_destroy();
}
