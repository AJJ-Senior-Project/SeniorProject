#ifndef SENDERWORKER_H
#define SENDERWORKER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <atomic>
#include <string>

class SenderWorker : public QObject
{
    Q_OBJECT
public:
    explicit SenderWorker(NDIlib_send_instance_t ndiSendInstance, QObject *parent = nullptr);
    ~SenderWorker();

public slots:
    void startScreenCapture();
    void startCameraFeed(const std::string &cameraID);
    void startAudioFeed(const std::string &audioSource);
    void sendMessage(const std::string &message, int priority, const std::string &game);
    void stopAll();

private:
    void captureScreen();
    void captureCameraFeed(const std::string &cameraID);
    void captureAudioFeed(const std::string &audioSource);

    NDIlib_send_instance_t ndiSendInstance;
    std::atomic<bool> sendingScreen;
    std::atomic<bool> sendingCamera;
    std::atomic<bool> sendingAudio;
};

#endif // SENDERWORKER_H
