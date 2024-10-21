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

    // Assume the video resolution is 1920x1080 (Full HD)
    const int width = 1920;
    const int height = 1080;

    // Generate and send video frames over NDI (example with color frames)
    // We'll simulate a video with different solid colors in this example
    for (int i = 0; i < 100; ++i) {
        // Create an NDI video frame
        NDIlib_video_frame_v2_t ndiVideoFrame;
        ndiVideoFrame.xres = width;
        ndiVideoFrame.yres = height;
        ndiVideoFrame.FourCC = NDIlib_FourCC_type_RGBX; // Use RGBX pixel format
        ndiVideoFrame.frame_rate_N = 30000; // 30fps (30,000/1000)
        ndiVideoFrame.frame_rate_D = 1000;

        // Allocate memory for the video frame (width * height * 4 for RGBX)
        unsigned char* videoData = new unsigned char[width * height * 4];

        // Fill the frame with a solid color that changes every frame (e.g., red, green, blue)
        unsigned char colorValue = static_cast<unsigned char>((i * 2) % 255); // Cycle through color values
        for (int pixel = 0; pixel < width * height; ++pixel) {
            videoData[pixel * 4] = colorValue;     // R
            videoData[pixel * 4 + 1] = 0;          // G
            videoData[pixel * 4 + 2] = 0;          // B
            videoData[pixel * 4 + 3] = 255;        // A (Alpha)
        }

        // Set the video frame data
        ndiVideoFrame.p_data = videoData;
        ndiVideoFrame.line_stride_in_bytes = width * 4; // Line stride in bytes for RGBX format

        // Send the video frame
        NDIlib_send_send_video_v2(ndiSend_, &ndiVideoFrame);

        // Simulate frame interval (e.g., 30fps -> ~33ms per frame)
        std::this_thread::sleep_for(std::chrono::milliseconds(33));

        // Clean up frame data after sending
        delete[] videoData;
    }

    std::cout << "Completed sending video from source: " << currentVideoSource_ << std::endl;
    return true;
}


// Send camera feed (by camera ID)
bool NDISender::sendCameraFeed(const std::string& cameraID) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Get the list of available cameras
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();

    // Check if any cameras are available
    if (availableCameras.isEmpty()) {
        std::cerr << "No camera devices found." << std::endl;
        return false;
    }

    // Find the camera matching the given cameraID (as a camera name)
    QCameraDevice selectedCamera;
    for (const QCameraDevice &camera : availableCameras) {
        if (camera.description().toStdString() == cameraID) {
            selectedCamera = camera;
            break;
        }
    }

    if (selectedCamera.isNull()) {
        std::cerr << "Camera with ID " << cameraID << " not found." << std::endl;
        return false;
    }

    // Set up the camera and session
    QCamera* camera = new QCamera(selectedCamera);
    QMediaCaptureSession* captureSession = new QMediaCaptureSession();
    captureSession->setCamera(camera);

    // Set up a video sink to capture frames
    QVideoSink* videoSink = new QVideoSink();
    captureSession->setVideoSink(videoSink);

    // Connect to the new video frame signal
    QObject::connect(videoSink, &QVideoSink::videoFrameChanged, [this](const QVideoFrame &frame) {
        if (!ndiSend_) {
            std::cerr << "NDI Sender not initialized." << std::endl;
            return;
        }

        if (!frame.isValid()) {
            std::cerr << "Invalid video frame." << std::endl;
            return;
        }

        // Map the frame to access pixel data
        QVideoFrame mappedFrame = frame;
        if (!mappedFrame.map(QVideoFrame::ReadOnly)) {
            std::cerr << "Failed to map video frame." << std::endl;
            return;
        }

        // For RGB formats, we use plane 0 (single plane for RGB)
        const uchar* frameData = mappedFrame.bits(0); // Specify the plane index
        if (!frameData) {
            std::cerr << "Failed to retrieve video frame data." << std::endl;
            mappedFrame.unmap();
            return;
        }

        NDIlib_video_frame_v2_t ndiVideoFrame;
        ndiVideoFrame.xres = mappedFrame.width();
        ndiVideoFrame.yres = mappedFrame.height();
        ndiVideoFrame.FourCC = NDIlib_FourCC_type_RGBX;  // RGBX format
        ndiVideoFrame.p_data = const_cast<uchar*>(frameData); // Pass frame data to NDI
        ndiVideoFrame.line_stride_in_bytes = mappedFrame.bytesPerLine(0); // Use plane 0's stride
        ndiVideoFrame.frame_rate_N = 30000;  // 30fps numerator
        ndiVideoFrame.frame_rate_D = 1000;   // 30fps denominator

        // Send the frame over NDI
        NDIlib_send_send_video_v2(ndiSend_, &ndiVideoFrame);

        std::cout << "Camera frame sent." << std::endl;

        // Unmap the frame after use
        mappedFrame.unmap();
    });

    // Start the camera
    camera->start();

    std::cout << "Camera feed initialized successfully for ID: " << cameraID << std::endl;

    return true;
}

// Send audio data
bool NDISender::sendAudio(const std::string& audioSource) {
    if (!ndiSend_) {
        std::cerr << "NDI Sender not initialized." << std::endl;
        return false;
    }

    // Open the audio source (e.g., a raw PCM or WAV file)
    std::ifstream audioFile(audioSource, std::ios::binary);
    if (!audioFile.is_open()) {
        std::cerr << "Failed to open audio source: " << audioSource << std::endl;
        return false;
    }

    // Buffer to hold the audio data
    const int bufferSize = 48000 * 2; // Example: 48kHz stereo audio, 16-bit PCM
    char audioBuffer[bufferSize];

    // Simulating continuous audio sending in chunks
    while (!audioFile.eof()) {
        // Read a chunk of audio data into the buffer
        audioFile.read(audioBuffer, bufferSize);
        std::streamsize bytesRead = audioFile.gcount();

        if (bytesRead <= 0) {
            break;
        }

        // Create an NDI audio frame
        NDIlib_audio_frame_v2_t ndiAudioFrame;
        ndiAudioFrame.sample_rate = 48000;        // Example: 48kHz sample rate
        ndiAudioFrame.no_channels = 2;            // Stereo audio (2 channels)
        ndiAudioFrame.no_samples = bytesRead / 4; // Number of samples in this frame (16-bit stereo = 4 bytes per sample)
        ndiAudioFrame.p_data = (float*)audioBuffer; // Audio data

        // Send the audio frame
        NDIlib_send_send_audio_v2(ndiSend_, &ndiAudioFrame);

        std::cout << "Audio frame sent with " << bytesRead << " bytes." << std::endl;

        // Simulate a delay based on the audio sample rate (e.g., for 48kHz audio)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Completed sending audio from source: " << audioSource << std::endl;
    return true;
}
