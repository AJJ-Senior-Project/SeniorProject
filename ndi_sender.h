#ifndef NDI_SENDER_H
#define NDI_SENDER_H

#include <windows.h>  // Add this line to resolve 'HWND' error
#include <cstddef>    // For NULL definition
#include <iostream>   // Include other necessary headers
#include <QObject>
#include <QStringList>
#include <QThread>
#include <Processing.NDI.Lib.h>
#include <string>
#include <mutex>
#include "senderworker.h"

class NDISender : public QObject
{
    Q_OBJECT
public:
    explicit NDISender(QObject *parent = nullptr);
    ~NDISender();

    bool initializeNDI();
    void startScreenCapture();
    void startCameraFeed(const std::string &cameraID);
    void startAudioFeed(const std::string &audioSource);
    void sendMessage(const std::string &message, int priority, const std::string &game);
    void stopAll();

private:
    bool initialized_ = false;
    NDIlib_send_instance_t ndiSend_;
    QThread *workerThread;
    SenderWorker *senderWorker;
};

#endif // NDI_SENDER_H
