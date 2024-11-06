#include "ndi_receiver.h"

NDIReceiver::NDIReceiver() : ndiFindInstance(nullptr), receiving(false) {}

NDIReceiver::~NDIReceiver()
{
    terminateReceiver();
}

// Initialize the NDI receiver
bool NDIReceiver::initializeReceiver()
{
    if (!NDIlib_initialize()) {
        std::cerr << "Failed to initialize NDI." << std::endl;
        return false;
    }

    // Create a finder instance to search for sources
    ndiFindInstance = NDIlib_find_create_v2();
    if (!ndiFindInstance) {
        std::cerr << "Failed to create NDI finder." << std::endl;
        return false;
    }

    std::cout << "NDI Receiver initialized successfully." << std::endl;
    return true;
}

// Discover available NDI sources
void NDIReceiver::discoverSources()
{
    if (!ndiFindInstance) {
        std::cerr << "NDI finder not initialized." << std::endl;
        return;
    }

    // Wait for up to 5 seconds to discover sources
    NDIlib_find_wait_for_sources(ndiFindInstance, 5000);
    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(ndiFindInstance, &numSources);

    std::lock_guard<std::mutex> lock(sourcesMutex); // Lock mutex to safely update availableSources
    availableSources.clear();
    for (uint32_t i = 0; i < numSources; ++i) {
        availableSources.push_back(sources[i]);
    }

    printAvailableSources();
}

// Print available NDI sources to the console
void NDIReceiver::printAvailableSources()
{
    std::cout << "Available NDI sources:" << std::endl;
    for (const auto& source : availableSources) {
        std::cout << "  - " << source.p_ndi_name << std::endl;
    }
}

// Start receiving video and metadata for all discovered sources
void NDIReceiver::startReceiving()
{
    std::lock_guard<std::mutex> lock(sourcesMutex); // Lock mutex to safely access availableSources
    receiving = true;

    for (const auto& source : availableSources) {
        // Create a receiver instance for each source and store it
        NDIlib_recv_instance_t instance = NDIlib_recv_create_v3();
        if (!instance) {
            std::cerr << "Failed to create NDI receiver instance for " << source.p_ndi_name << std::endl;
            continue;
        }
        NDIlib_recv_connect(instance, &source);
        ndiReceiverInstances[source.p_ndi_name] = instance;

        // Create a new thread for each source to receive video and metadata
        receiverThreads[source.p_ndi_name] = std::thread(&NDIReceiver::receiveVideoAndMetadata, this, source);
    }
}

// Receive video and metadata for a specific source (used in a separate thread for each source)
void NDIReceiver::receiveVideoAndMetadata(const NDIlib_source_t& source)
{
    auto instance = ndiReceiverInstances[source.p_ndi_name];
    if (!instance) {
        std::cerr << "NDI Receiver instance not initialized for " << source.p_ndi_name << std::endl;
        return;
    }

    while (receiving) {
        NDIlib_video_frame_v2_t videoFrame;
        NDIlib_metadata_frame_t metadataFrame;

        switch (NDIlib_recv_capture_v2(instance, &videoFrame, nullptr, &metadataFrame, 5000)) {
        case NDIlib_frame_type_video:
            std::cout << "Received video frame from source: " << source.p_ndi_name << std::endl;
            NDIlib_recv_free_video_v2(instance, &videoFrame);
            break;

        case NDIlib_frame_type_metadata:
            std::cout << "Received metadata from " << source.p_ndi_name << ": " << metadataFrame.p_data << std::endl;
            NDIlib_recv_free_metadata(instance, &metadataFrame);
            break;

        case NDIlib_frame_type_none:
            std::cerr << "No data received from source: " << source.p_ndi_name << std::endl;
            break;

        default:
            std::cerr << "Error receiving data from source: " << source.p_ndi_name << std::endl;
            break;
        }
    }
}

// Terminate the NDI receiver, stop all threads, and free resources
void NDIReceiver::terminateReceiver()
{
    receiving = false; // Signal threads to stop

    // Join each thread and clean up instances
    for (auto& threadPair : receiverThreads) {
        if (threadPair.second.joinable()) {
            threadPair.second.join();
        }
    }
    receiverThreads.clear();

    // Destroy each NDI receiver instance
    for (auto& instancePair : ndiReceiverInstances) {
        NDIlib_recv_destroy(instancePair.second);
    }
    ndiReceiverInstances.clear();

    if (ndiFindInstance) {
        NDIlib_find_destroy(ndiFindInstance);
        ndiFindInstance = nullptr;
    }

    NDIlib_destroy();
    std::cout << "NDI Receiver terminated successfully." << std::endl;
}
