#include "senderworker.h"
#include <QDebug>
#include <windows.h>
#include <wingdi.h>
#include <chrono>
#include <thread>
#include <QAudioFormat>
#include <QVideoFrame>
#include <QImage>
#include <QVideoFrameFormat>

SenderWorker::SenderWorker(NDIlib_send_instance_t ndiSendCameraMicInstance,
                           NDIlib_send_instance_t ndiSendScreenShareInstance,
                           QObject *parent)
    : QObject(parent),
    ndiSendCameraMicInstance_(ndiSendCameraMicInstance),
    ndiSendScreenShareInstance_(ndiSendScreenShareInstance),
    targetWindowHandle_(nullptr),
    sending_(false),
    screenCaptureTimer_(nullptr),
    audioCaptureTimer_(nullptr),
    camera_(nullptr),
    captureSession_(nullptr),
    videoSink_(nullptr),
    audioInput_(nullptr),
    audioIO_(nullptr)
{
}

SenderWorker::~SenderWorker()
{
    stopAll();
}

void SenderWorker::start(const QString &cameraID, const QString &audioSource, const QString &applicationName)
{
    sending_ = true;

    // Start camera feed
    startCameraFeed(cameraID);

    // Start audio feed
    startAudioFeed(audioSource);

    // Start screen capture
    startScreenCapture(applicationName);
}

void SenderWorker::startScreenCapture(const QString &applicationName)
{
    qDebug() << "SenderWorker: Starting screen capture for application:" << applicationName;

    // Find the window handle (HWND) for the application
    targetWindowHandle_ = nullptr;

    struct FindWindowData {
        QString targetWindowTitle;
        HWND hwnd;
    };

    FindWindowData data;
    data.targetWindowTitle = applicationName;
    data.hwnd = nullptr;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        FindWindowData *data = reinterpret_cast<FindWindowData*>(lParam);
        TCHAR windowTitle[MAX_PATH];
        GetWindowText(hwnd, windowTitle, MAX_PATH);
        if (IsWindowVisible(hwnd) && wcslen(windowTitle) > 0) {
            QString currentWindowTitle = QString::fromWCharArray(windowTitle);
            if (currentWindowTitle == data->targetWindowTitle) {
                data->hwnd = hwnd;
                return FALSE; // Stop enumeration
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));

    if (!data.hwnd) {
        qWarning() << "Could not find window for application:" << applicationName;
        return;
    }

    targetWindowHandle_ = data.hwnd;
    qDebug() << "Found window handle for application.";

    // Start a timer to capture screen periodically
    screenCaptureTimer_ = new QTimer(this);
    connect(screenCaptureTimer_, &QTimer::timeout, this, &SenderWorker::captureScreen);
    screenCaptureTimer_->start(33); // Approximately 30 fps
}

void SenderWorker::startCameraFeed(const QString &cameraID)
{
    qDebug() << "SenderWorker: Starting camera feed with camera ID:" << cameraID;

    // Find the camera by ID
    QCameraDevice selectedCamera;
    auto cameras = QMediaDevices::videoInputs();
    for (const auto &cam : cameras) {
        if (cam.id() == cameraID) {
            selectedCamera = cam;
            break;
        }
    }

    if (!selectedCamera.isNull()) {
        camera_ = new QCamera(selectedCamera);
        captureSession_ = new QMediaCaptureSession();
        videoSink_ = new QVideoSink();

        captureSession_->setCamera(camera_);
        captureSession_->setVideoSink(videoSink_);

        connect(videoSink_, &QVideoSink::videoFrameChanged, this, &SenderWorker::onCameraFrameReceived);

        camera_->start();
        qDebug() << "Camera started.";
    } else {
        qWarning() << "Selected camera not found.";
    }
}

void SenderWorker::onCameraFrameReceived(const QVideoFrame &frame)
{
    if (!ndiSendCameraMicInstance_) return;

    if (!frame.isValid()) {
        qWarning() << "Invalid video frame received.";
        return;
    }

    QImage image = frame.toImage();
    if (image.isNull()) {
        qWarning() << "Received invalid image from camera.";
        return;
    }

    // Convert image to ARGB32 format if necessary
    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    // Create a copy of the image data
    QByteArray imageData((const char*)image.bits(), image.sizeInBytes());

    NDIlib_video_frame_v2_t ndiFrame;
    ndiFrame.xres = image.width();
    ndiFrame.yres = image.height();
    ndiFrame.FourCC = NDIlib_FourCC_type_BGRA;
    ndiFrame.frame_rate_N = 30000;
    ndiFrame.frame_rate_D = 1001;
    ndiFrame.picture_aspect_ratio = static_cast<float>(image.width()) / static_cast<float>(image.height());
    ndiFrame.line_stride_in_bytes = image.bytesPerLine();
    ndiFrame.p_data = reinterpret_cast<uint8_t*>(imageData.data());

    NDIlib_send_send_video_v2(ndiSendCameraMicInstance_, &ndiFrame);
}

void SenderWorker::startAudioFeed(const QString &audioSource)
{
    qDebug() << "SenderWorker: Starting audio feed with audio source:" << audioSource;

    QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (!audioSource.isEmpty()) {
        auto devices = QMediaDevices::audioInputs();
        for (const auto &dev : devices) {
            if (dev.id() == audioSource) {
                device = dev;
                break;
            }
        }
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!device.isFormatSupported(format)) {
        qWarning() << "Requested audio format not supported. Trying preferred format.";
        format = device.preferredFormat();
        if (format.sampleFormat() != QAudioFormat::Int16) {
            qWarning() << "Preferred format is not Int16. Audio feed may not work properly.";
        }
    }

    audioInput_ = new QAudioSource(device, format, this);
    audioIO_ = audioInput_->start();

    connect(audioIO_, &QIODevice::readyRead, this, [this]() {
        QMutexLocker locker(&audioMutex_);
        QByteArray data = audioIO_->readAll();
        audioBuffer_.append(data);
    });

    // Start a timer to send audio frames periodically
    audioCaptureTimer_ = new QTimer(this);
    connect(audioCaptureTimer_, &QTimer::timeout, this, &SenderWorker::captureAudioFeed);
    audioCaptureTimer_->start(20); // Adjust as needed
}

void SenderWorker::captureAudioFeed()
{
    if (!ndiSendCameraMicInstance_) return;

    QMutexLocker locker(&audioMutex_);
    if (audioBuffer_.isEmpty()) return;

    QByteArray audioData = audioBuffer_;
    audioBuffer_.clear();

    int numChannels = 2; // Assuming stereo audio
    int totalSamples = audioData.size() / sizeof(int16_t);
    int numSamples = totalSamples / numChannels;

    const int16_t* int16Data = reinterpret_cast<const int16_t*>(audioData.constData());

    // Convert int16_t samples to float samples
    std::vector<float> floatBuffer(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
        floatBuffer[i] = int16Data[i] / 32768.0f;
    }

    NDIlib_audio_frame_v2_t audioFrame;
    audioFrame.sample_rate = 48000;
    audioFrame.no_channels = numChannels;
    audioFrame.no_samples = numSamples;
    audioFrame.p_data = floatBuffer.data();
    audioFrame.channel_stride_in_bytes = numSamples * sizeof(float);

    NDIlib_send_send_audio_v2(ndiSendCameraMicInstance_, &audioFrame);
}

void SenderWorker::stopAll()
{
    sending_ = false;

    if (screenCaptureTimer_) {
        screenCaptureTimer_->stop();
        screenCaptureTimer_->deleteLater();
        screenCaptureTimer_ = nullptr;
    }

    if (audioCaptureTimer_) {
        audioCaptureTimer_->stop();
        audioCaptureTimer_->deleteLater();
        audioCaptureTimer_ = nullptr;
    }

    if (camera_) {
        camera_->stop();
        delete captureSession_;
        captureSession_ = nullptr;
        delete camera_;
        camera_ = nullptr;
        delete videoSink_;
        videoSink_ = nullptr;
        qDebug() << "Camera feed stopped.";
    }

    if (audioInput_) {
        audioInput_->stop();
        delete audioInput_;
        audioInput_ = nullptr;
        audioIO_ = nullptr;
        qDebug() << "Audio feed stopped.";
    }

    emit finished();
}

void SenderWorker::captureScreen()
{
    if (!ndiSendScreenShareInstance_ || !targetWindowHandle_) return;

    HDC hWindowDC = GetDC(targetWindowHandle_);
    HDC hMemoryDC = CreateCompatibleDC(hWindowDC);

    RECT rect;
    GetClientRect(targetWindowHandle_, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    HBITMAP hBitmap = CreateCompatibleBitmap(hWindowDC, windowWidth, windowHeight);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, windowWidth, windowHeight, hWindowDC, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = windowWidth;
    bi.biHeight = -windowHeight;  // Negative height to correct image orientation
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    int imageSize = windowWidth * windowHeight * 4;
    std::vector<unsigned char> buffer(imageSize);
    GetDIBits(hMemoryDC, hBitmap, 0, windowHeight, buffer.data(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    NDIlib_video_frame_v2_t ndiVideoFrame;
    ndiVideoFrame.xres = windowWidth;
    ndiVideoFrame.yres = windowHeight;
    ndiVideoFrame.FourCC = NDIlib_FourCC_type_BGRA;
    ndiVideoFrame.p_data = buffer.data();
    ndiVideoFrame.line_stride_in_bytes = windowWidth * 4;
    ndiVideoFrame.frame_rate_N = 30000;
    ndiVideoFrame.frame_rate_D = 1001;
    ndiVideoFrame.picture_aspect_ratio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    NDIlib_send_send_video_v2(ndiSendScreenShareInstance_, &ndiVideoFrame);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(targetWindowHandle_, hWindowDC);
}
