#ifndef NDI_SENDER_H
#define NDI_SENDER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QThread>
#include <QString>
#include "senderworker.h"

class NDISender : public QObject
{
    Q_OBJECT
public:
    explicit NDISender(QObject *parent = nullptr);
    ~NDISender();

    bool initializeNDI();
    void sendMessage(const QString &message, int priority, const QString &application);
    void startAllStreams(const QString &cameraID, const QString &audioSource, const QString &applicationName);

    void stopAll();

private:
    bool initialized_;
    NDIlib_send_instance_t ndiSendCameraMic_;
    NDIlib_send_instance_t ndiSendScreenShare_;
    QThread *workerThread_;
    SenderWorker *senderWorker_;
};

#endif // NDI_SENDER_H
