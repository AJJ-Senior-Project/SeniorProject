#ifndef RECEIVERWORKER_H
#define RECEIVERWORKER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QImage>
#include <QTimer>

class ReceiverWorker : public QObject
{
    Q_OBJECT
public:
    explicit ReceiverWorker(NDIlib_recv_instance_t ndiReceiverInstance, QObject *parent = nullptr);
    ~ReceiverWorker();

signals:
    void frameReceived(const QImage &frame);
    void finished();

public slots:
    void start();
    void stop();

private slots:
    void captureFrame();

private:
    NDIlib_recv_instance_t ndiReceiverInstance;
    QTimer *captureTimer;
};

#endif // RECEIVERWORKER_H
