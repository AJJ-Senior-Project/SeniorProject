#include "ndi_sender.h"
#include <QDebug>
#include <iostream>
#include <sstream>
#include <thread>

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

// Initialize NDI resources
bool NDISender::initializeNDI() {
    // Initialize NDI library
    if (!NDIlib_initialize()) {
        std::cerr << "Failed to initialize NDI." << std::endl;
        return false;
    }

    // Create an NDI sender instance
    NDIlib_send_create_t NDI_send_create_desc;
    NDI_send_create_desc.p_ndi_name = "NDI Sender";
    ndiSend_ = NDIlib_send_create(&NDI_send_create_desc);

    if (!ndiSend_) {
        std::cerr << "Failed to create NDI sender instance." << std::endl;
        return false;
    }

    std::cout << "NDI Sender initialized successfully." << std::endl;
    return true;
}

// Terminate NDI resources
void NDISender::terminateNDI() {
    if (ndiSend_) {
        NDIlib_send_destroy(ndiSend_);
        NDIlib_destroy();
        ndiSend_ = nullptr;
        std::cout << "NDI Sender terminated successfully." << std::endl;
    }
}

// Set the priority of messages
void NDISender::setPriority(int priority) {
    priority_ = priority;
}

// Send a message (string) over NDI
bool NDISender::sendMessage(const std::string& message, int priority, const std::string& game) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Create JSON format
    std::stringstream jsonMessage;
    jsonMessage << "{"
                << "\"priority\": " << priority << ","
                << "\"message\": \"" << message << "\","
                << "\"game\": \"" << game << "\""
                << "}";

    // Convert string to char*
    const char* jsonCString = jsonMessage.str().c_str();

    // Send the message as metadata
    NDIlib_metadata_frame_t metadata_frame;
    metadata_frame.p_data = const_cast<char*>(jsonCString);
    metadata_frame.length = strlen(jsonCString);

    NDIlib_send_send_metadata(ndiSend_, &metadata_frame);

    std::cout << "Message sent: " << jsonMessage.str() << std::endl;
    return true;
}

// Send video from a file source
bool NDISender::sendVideoSource(const std::string& sourcePath) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    currentVideoSource_ = sourcePath;
    // In real-world use, you would open the video file and send frames over NDI.
    // This is a placeholder for video sending logic.
    std::cout << "Sending video from source: " << currentVideoSource_ << std::endl;

    // Simulate sending video over NDI
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Simulate sending delay

    return true;
}

// Send camera feed (by camera ID)
bool NDISender::sendCameraFeed(const std::string& cameraID) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Here, you would connect to the camera using the cameraID and send frames over NDI.
    // This is a placeholder for camera feed sending logic.
    std::cout << "Sending camera feed from camera ID: " << cameraID << std::endl;

    return true;
}

// Send audio data
bool NDISender::sendAudio(const std::string& audioSource) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Placeholder for audio sending logic. You would normally handle audio data and send it over NDI.
    std::cout << "Sending audio from source: " << audioSource << std::endl;

    return true;
}
