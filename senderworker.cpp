#include "senderworker.h"
#include <QDebug>
#include <windows.h>
#include <wingdi.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <cmath>

SenderWorker::SenderWorker(NDIlib_send_instance_t ndiSendInstance, QObject *parent)
    : QObject(parent), ndiSendInstance(ndiSendInstance), sendingScreen(false), sendingCamera(false), sendingAudio(false)
{
}

SenderWorker::~SenderWorker()
{
}

void SenderWorker::startScreenCapture()
{
    sendingScreen = true;
    qDebug() << "SenderWorker: Starting screen capture sending loop.";

    while (sendingScreen) {
        captureScreen();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30fps
    }

    qDebug() << "SenderWorker: Stopped sending screen capture.";
}

void SenderWorker::startCameraFeed(const std::string &cameraID)
{
    sendingCamera = true;
    qDebug() << "SenderWorker: Starting camera feed sending loop.";

    while (sendingCamera) {
        captureCameraFeed(cameraID);
        std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30fps
    }

    qDebug() << "SenderWorker: Stopped sending camera feed.";
}

void SenderWorker::startAudioFeed(const std::string &audioSource)
{
    sendingAudio = true;
    qDebug() << "SenderWorker: Starting audio feed sending loop.";

    while (sendingAudio) {
        captureAudioFeed(audioSource);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Adjust rate as needed
    }

    qDebug() << "SenderWorker: Stopped sending audio feed.";
}

void SenderWorker::sendMessage(const std::string &message, int priority, const std::string &game)
{
    if (!ndiSendInstance) return;

    std::stringstream jsonMessage;
    jsonMessage << "{\"priority\":" << priority << ",\"message\":\"" << message
                << "\",\"game\":\"" << game << "\"}";

    NDIlib_metadata_frame_t metadataFrame;
    metadataFrame.p_data = const_cast<char *>(jsonMessage.str().c_str());
    metadataFrame.length = strlen(jsonMessage.str().c_str());

    NDIlib_send_send_metadata(ndiSendInstance, &metadataFrame);
}

void SenderWorker::stopAll()
{
    sendingScreen = false;
    sendingCamera = false;
    sendingAudio = false;
}

// Screen Capture Implementation
void SenderWorker::captureScreen()
{
    if (!ndiSendInstance) return;

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
    int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight;  // Negative height to correct image orientation
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    int imageSize = screenWidth * screenHeight * 4;
    unsigned char *buffer = new unsigned char[imageSize];
    GetDIBits(hMemoryDC, hBitmap, 0, screenHeight, buffer, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    NDIlib_video_frame_v2_t ndiVideoFrame;
    ndiVideoFrame.xres = screenWidth;
    ndiVideoFrame.yres = screenHeight;
    ndiVideoFrame.FourCC = NDIlib_FourCC_type_RGBX;
    ndiVideoFrame.p_data = buffer;

    NDIlib_send_send_video_v2(ndiSendInstance, &ndiVideoFrame);

    delete[] buffer;
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

// Camera Feed Capture Implementation
void SenderWorker::captureCameraFeed(const std::string &cameraID)
{
    if (!ndiSendInstance) return;

    const int width = 1280;
    const int height = 720;
    const int frameRate = 30;

    NDIlib_video_frame_v2_t videoFrame;
    videoFrame.xres = width;
    videoFrame.yres = height;
    videoFrame.FourCC = NDIlib_FourCC_type_RGBX;
    videoFrame.frame_rate_N = frameRate * 1000;
    videoFrame.frame_rate_D = 1000;
    videoFrame.line_stride_in_bytes = width * 4;

    unsigned char *videoData = new unsigned char[width * height * 4];
    unsigned char colorValue = static_cast<unsigned char>(rand() % 255);  // Random color value for testing
    for (int pixel = 0; pixel < width * height; ++pixel) {
        videoData[pixel * 4] = colorValue;
        videoData[pixel * 4 + 1] = 0;
        videoData[pixel * 4 + 2] = 0;
        videoData[pixel * 4 + 3] = 255;
    }

    videoFrame.p_data = videoData;
    NDIlib_send_send_video_v2(ndiSendInstance, &videoFrame);
    delete[] videoData;
}

// Audio Feed Capture Implementation
void SenderWorker::captureAudioFeed(const std::string &audioSource)
{
    if (!ndiSendInstance) return;

    const int sampleRate = 48000;
    const int numChannels = 2;
    const int samplesPerFrame = 1600;

    float *audioData = new float[samplesPerFrame * numChannels];

    for (int sample = 0; sample < samplesPerFrame; ++sample) {
        float value = 0.1f * std::sin(2.0f * 3.14159f * sample / 100.0f);
        audioData[sample * numChannels] = value;
        audioData[sample * numChannels + 1] = value;
    }

    NDIlib_audio_frame_v2_t audioFrame;
    audioFrame.sample_rate = sampleRate;
    audioFrame.no_channels = numChannels;
    audioFrame.no_samples = samplesPerFrame;
    audioFrame.p_data = audioData;
    audioFrame.channel_stride_in_bytes = samplesPerFrame * sizeof(float);

    NDIlib_send_send_audio_v2(ndiSendInstance, &audioFrame);
    delete[] audioData;
}
