#ifndef NDI_SENDER_H
#define NDI_SENDER_H

#include <cstddef>  // For NULL definition, if necessary
#include <iostream> // Include other necessary headers
#include <Processing.NDI.Lib.h>
#include <QObject>
#include <QDebug>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <QCamera>
#include <QAudioInput>
#include <QAudioFormat>
#include <QVideoFrame>
#include <QIODevice>
#include <QByteArray>
#include <QImage>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QTimer>
#include <QVideoSink>
#include <QBuffer>
#include <iostream>

class NDISender : public QObject{
    Q_OBJECT
public:
    NDISender();
    ~NDISender();
    void initializeSender();
    void sendFrame(const QImage &frame);

    //added 10/15/24
    bool sendMessage(const std::string& message, int priority, const std::string& game);
    bool sendVideoSource(const std::string& sourcePath);
    bool sendCameraFeed(const std::string& cameraID);
    bool sendAudio(const std::string& audioSource);
    void setPriority(int priority);  // Set message priority
    bool initializeNDI();            // Initialize NDI resources
    void terminateNDI();             // Clean up NDI resources
    static bool IsMainApplicationWindow(HWND hWnd);
    QStringList getRunningApplications();

private:
    //added 10/15/24
    int priority_;                     // Priority of messages
    std::string game_;                 // Game metadata
    NDIlib_send_instance_t ndiSend_;   // NDI sender instance
    std::string currentVideoSource_;   // Path to current video source
    NDIlib_send_instance_t ndi_send_instance;
};



#endif // NDI_SENDER_H
