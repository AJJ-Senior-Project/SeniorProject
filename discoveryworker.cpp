#include "discoveryworker.h"
#include <iostream>

DiscoveryWorker::DiscoveryWorker(NDIlib_find_instance_t ndiFindInstance, QObject *parent)
    : QObject(parent), ndiFindInstance(ndiFindInstance)
{
}

DiscoveryWorker::~DiscoveryWorker(){}

void DiscoveryWorker::process()
{
    if (!ndiFindInstance) {
        std::cerr << "NDI finder not initialized." << std::endl;
        emit sourcesDiscovered(QStringList());
        return;
    }

    // Wait for up to 5 seconds to discover sources
    NDIlib_find_wait_for_sources(ndiFindInstance, 5000);

    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(ndiFindInstance, &numSources);

    QStringList sourceNames;
    for (uint32_t i = 0; i < numSources; ++i) {
        sourceNames.append(QString::fromUtf8(sources[i].p_ndi_name));
    }

    emit sourcesDiscovered(sourceNames);
}
