#ifndef RECEIVERWORKER_H
#define RECEIVERWORKER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QImage>
#include <atomic>

class ReceiverWorker : public QObject
{
    Q_OBJECT
public:
    explicit ReceiverWorker(NDIlib_recv_instance_t ndiReceiverInstance, QObject *parent = nullptr);
    ~ReceiverWorker();

signals:
    void frameReceived(const QImage &frame);

public slots:
    void process();
    void stop();

private:
    NDIlib_recv_instance_t ndiReceiverInstance;
    std::atomic<bool> receiving;
};

#endif // RECEIVERWORKER_H
