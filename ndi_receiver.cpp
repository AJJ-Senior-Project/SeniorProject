#include "ndi_receiver.h"

NDIReceiver::NDIReceiver() : ndiFindInstance(nullptr), ndiReceiverInstance(nullptr) {}

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

// Select a specific NDI source by name
bool NDIReceiver::selectSource(const std::string& sourceName)
{
    for (const auto& source : availableSources) {
        if (sourceName == source.p_ndi_name) {
            // Create a receiver instance for the selected source
            ndiReceiverInstance = NDIlib_recv_create_v3();
            if (!ndiReceiverInstance) {
                std::cerr << "Failed to create NDI receiver instance." << std::endl;
                return false;
            }

            // Connect the receiver to the selected source
            NDIlib_recv_connect(ndiReceiverInstance, &source);
            std::cout << "Connected to source: " << sourceName << std::endl;
            return true;
        }
    }

    std::cerr << "Source not found: " << sourceName << std::endl;
    return false;
}

// Receive video and metadata from the selected source
void NDIReceiver::receiveVideoAndMetadata()
{
    if (!ndiReceiverInstance) {
        std::cerr << "NDI Receiver instance not initialized." << std::endl;
        return;
    }

    while (true) {
        // Receive video frame
        NDIlib_video_frame_v2_t videoFrame;
        NDIlib_metadata_frame_t metadataFrame;

        switch (NDIlib_recv_capture_v2(ndiReceiverInstance, &videoFrame, nullptr, &metadataFrame, 5000)) {
        case NDIlib_frame_type_video:
            std::cout << "Received video frame from source." << std::endl;
            // Process video frame (this is where you would package or display it)
            NDIlib_recv_free_video_v2(ndiReceiverInstance, &videoFrame);
            break;

        case NDIlib_frame_type_metadata:
            std::cout << "Received metadata: " << metadataFrame.p_data << std::endl;
            NDIlib_recv_free_metadata(ndiReceiverInstance, &metadataFrame);
            break;

        case NDIlib_frame_type_none:
            std::cerr << "No data received." << std::endl;
            break;

        default:
            std::cerr << "Error receiving data." << std::endl;
            break;
        }
    }
}

// Terminate the NDI receiver and free resources
void NDIReceiver::terminateReceiver()
{
    if (ndiReceiverInstance) {
        NDIlib_recv_destroy(ndiReceiverInstance);
        ndiReceiverInstance = nullptr;
    }

    if (ndiFindInstance) {
        NDIlib_find_destroy(ndiFindInstance);
        ndiFindInstance = nullptr;
    }

    NDIlib_destroy();
    std::cout << "NDI Receiver terminated successfully." << std::endl;
}
