#include "ndi_sender.h"
#include <QDebug>
#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <QStringList>
#include <wingdi.h>

NDISender::NDISender() {
    if (!NDIlib_initialize()) {
        qDebug() << "Failed to initialize NDI library.";
        return;
    }

    NDIlib_send_create_t NDI_send_create_desc = {0};
    NDI_send_create_desc.p_ndi_name = "NDI Test Sender";
    ndiSend_ = NDIlib_send_create(&NDI_send_create_desc);

    if (!ndiSend_) {
        qDebug() << "Failed to create NDI sender instance.";
        NDIlib_destroy();
    } else {
        qDebug() << "NDI sender instance created successfully and is non-null.";
    }
}

NDISender::~NDISender()
{
    if (ndiSend_)
    {
        NDIlib_send_destroy(ndiSend_);
        ndiSend_ = nullptr; // Ensure no dangling pointer
    }
    NDIlib_destroy();
}

// Startup Occurs With this
void NDISender::initializeSender()
{
    // NDIlib_send_create_t NDI_send_create_desc = {0};
    // NDI_send_create_desc.p_ndi_name = "NDI Sender";
    // ndiSend_ = NDIlib_send_create(&NDI_send_create_desc);

    // if (ndiSend_ == nullptr) {
    //     qDebug() << "NDI sender instance is nullptr. Initialization failed.";
    // } else {
    //     qDebug() << "NDI sender instance created successfully and is non-null.";
    // }
}

// Initialize NDI resources
bool NDISender::initializeNDI() { //creates NDI object
    // Initialize the NDI library
    if (!NDIlib_initialize()) {
        std::cerr << "Failed to initialize NDI library." << std::endl;
        return false;
    } else {
    std::cout << "NDI library initialized successfully." << std::endl;
    return true;
    }
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


bool NDISender::sendVideoSource(const std::string& sourcePath) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Get the device context of the screen
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Get the screen dimensions
    int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
    int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);

    // Create a compatible bitmap from the Window DC
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    while (true) {
        // Copy the screen into the bitmap
        BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

        // Create a BITMAPINFOHEADER to access bitmap data
        BITMAPINFOHEADER bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = screenWidth;
        bi.biHeight = -screenHeight; // Negative to ensure the image isn't upside-down
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        // Retrieve bitmap data
        int imageSize = screenWidth * screenHeight * 4;
        unsigned char* buffer = new unsigned char[imageSize];
        GetDIBits(hMemoryDC, hBitmap, 0, screenHeight, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // Set up the NDI video frame
        NDIlib_video_frame_v2_t ndiVideoFrame;
        ndiVideoFrame.xres = screenWidth;
        ndiVideoFrame.yres = screenHeight;
        ndiVideoFrame.FourCC = NDIlib_FourCC_type_RGBX; // RGBX format
        ndiVideoFrame.frame_rate_N = 30000; // 30fps
        ndiVideoFrame.frame_rate_D = 1000;
        ndiVideoFrame.line_stride_in_bytes = screenWidth * 4;
        ndiVideoFrame.p_data = buffer;

        // Send the video frame
        NDIlib_send_send_video_v2(ndiSend_, &ndiVideoFrame);

        // Free the buffer after sending
        delete[] buffer;

        // Delay to match the frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    std::cout << "Completed sending desktop feed." << std::endl;
    return true;
}

// Camera and audio handling functions remain unchanged, except for adding thread safety.
bool NDISender::sendCameraFeed(const std::string& cameraID) {
    if (!ndiSend_) {
        qDebug() << "NDI sender not initialized. Cannot send camera feed.";
        return false;
    }

    // Placeholder camera feed dimensions and frame rate
    const int width = 1280;
    const int height = 720;
    const int frameRate = 30;

    // Loop to simulate sending frames from a camera feed
    for (int i = 0; i < 100; ++i) {  // Replace with actual frame capture loop

        // Create an NDI video frame
        NDIlib_video_frame_v2_t videoFrame;
        videoFrame.xres = width;
        videoFrame.yres = height;
        videoFrame.FourCC = NDIlib_FourCC_type_RGBX;
        videoFrame.frame_rate_N = frameRate * 1000;
        videoFrame.frame_rate_D = 1000;
        videoFrame.line_stride_in_bytes = width * 4;

        // Allocate memory for video data and set a color
        unsigned char* videoData = new unsigned char[width * height * 4];
        unsigned char colorValue = static_cast<unsigned char>((i * 2) % 255);
        for (int pixel = 0; pixel < width * height; ++pixel) {
            videoData[pixel * 4] = colorValue;     // R
            videoData[pixel * 4 + 1] = 0;          // G
            videoData[pixel * 4 + 2] = 0;          // B
            videoData[pixel * 4 + 3] = 255;        // A
        }

        // Set video frame data and send
        videoFrame.p_data = videoData;
        NDIlib_send_send_video_v2(ndiSend_, &videoFrame);

        // Free the frame data and sleep for the next frame
        delete[] videoData;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / frameRate));
    }

    qDebug() << "Camera feed sent successfully.";
    return true;
}

bool NDISender::sendAudio(const std::string& audioSource) {
    if (!ndiSend_) {
        qDebug() << "NDI sender not initialized. Cannot send audio.";
        return false;
    }

    const int sampleRate = 48000;  // Standard audio sample rate
    const int numChannels = 2;     // Stereo
    const int samplesPerFrame = 1600;  // Number of samples per frame

    // Create a buffer for audio data
    float* audioData = new float[samplesPerFrame * numChannels];

    // Simulate sending audio frames in a loop
    for (int i = 0; i < 100; ++i) {  // Replace with actual audio capture loop
        for (int sample = 0; sample < samplesPerFrame; ++sample) {
            float value = 0.1f * std::sin(2.0f * 3.14159f * sample / 100.0f);  // Placeholder sine wave
            audioData[sample * numChannels] = value;        // Left channel
            audioData[sample * numChannels + 1] = value;    // Right channel
        }

        // Create an NDI audio frame and send it
        NDIlib_audio_frame_v2_t audioFrame;
        audioFrame.sample_rate = sampleRate;
        audioFrame.no_channels = numChannels;
        audioFrame.no_samples = samplesPerFrame;
        audioFrame.p_data = audioData;
        audioFrame.channel_stride_in_bytes = samplesPerFrame * sizeof(float);

        NDIlib_send_send_audio_v2(ndiSend_, &audioFrame);
    }

    delete[] audioData;
    qDebug() << "Audio feed sent successfully.";
    return true;
}

bool NDISender::IsMainApplicationWindow(HWND hWnd) {
    return (IsWindowVisible(hWnd) && GetWindow(hWnd, GW_OWNER) == NULL);
}

// A static helper callback function for EnumWindows
static BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam) {
    if (NDISender::IsMainApplicationWindow(hWnd)) {
        TCHAR windowTitle[MAX_PATH];
        GetWindowText(hWnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

        // Only add if the window has a title (i.e., it's a user-facing window)
        if (wcslen(windowTitle) > 0) {
            auto appList = reinterpret_cast<QStringList*>(lParam);
            appList->append(QString::fromWCharArray(windowTitle));
        }
    }
    return TRUE; // Continue enumeration
}

QStringList NDISender::getRunningApplications() {
    QStringList appList;
    // Use the static callback function
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&appList));
    return appList;
}
