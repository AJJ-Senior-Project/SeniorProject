#include "ndi_sender.h"

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
        ndi_send_instance = nullptr; // Ensure no dangling pointer
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
        qDebug() << "Failed at Initialize";
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
        ndiSend_ = nullptr; // Prevent dangling pointer
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

    const int width = 1920;
    const int height = 1080;

    for (int i = 0; i < 100; ++i) {
        // Create an NDI video frame
        NDIlib_video_frame_v2_t ndiVideoFrame;
        ndiVideoFrame.xres = width;
        ndiVideoFrame.yres = height;
        ndiVideoFrame.FourCC = NDIlib_FourCC_type_RGBX; // Use RGBX pixel format
        ndiVideoFrame.frame_rate_N = 30000; // 30fps (30,000/1000)
        ndiVideoFrame.frame_rate_D = 1000;

        // Allocate memory for the video frame
        unsigned char* videoData = new unsigned char[width * height * 4];
        if (!videoData) {
            std::cerr << "Failed to allocate memory for video data." << std::endl;
            return false;
        }

        // Fill the frame with a solid color
        unsigned char colorValue = static_cast<unsigned char>((i * 2) % 255);
        for (int pixel = 0; pixel < width * height; ++pixel) {
            videoData[pixel * 4] = colorValue;
            videoData[pixel * 4 + 1] = 0;
            videoData[pixel * 4 + 2] = 0;
            videoData[pixel * 4 + 3] = 255;
        }

        // Set the video frame data
        ndiVideoFrame.p_data = videoData;
        ndiVideoFrame.line_stride_in_bytes = width * 4;

        // Send the video frame
        NDIlib_send_send_video_v2(ndiSend_, &ndiVideoFrame);

        std::this_thread::sleep_for(std::chrono::milliseconds(33));

        delete[] videoData;
    }

    std::cout << "Completed sending video from source: " << currentVideoSource_ << std::endl;
    return true;
}

// Camera and audio handling functions remain unchanged, except for adding thread safety.
