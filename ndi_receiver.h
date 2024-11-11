#ifndef NDI_RECEIVER_H
#define NDI_RECEIVER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QStringList>
#include <QThread>
#include <mutex>

class DiscoveryWorker;
class ReceiverWorker;

class NDIReceiver : public QObject
{
    Q_OBJECT
public:
    explicit NDIReceiver(QObject *parent = nullptr);
    ~NDIReceiver();

    bool initializeReceiver();
    void startDiscovery();
    void selectSource(const QString &sourceName);
    void startReceiving();
    void stopReceiving();
    void terminateReceiver();

signals:
    void sourcesDiscovered(const QStringList &sources);
    void frameReceived(const QImage &frame);

    void stopReceivingSignal();  // Signal to stop the receiver worker

private:
    NDIlib_find_instance_t ndiFindInstance;
    NDIlib_recv_instance_t ndiReceiverInstance;
    QThread *discoveryThread;
    QThread *receiverThread;
    DiscoveryWorker *discoveryWorker;
    ReceiverWorker *receiverWorker;
    std::vector<NDIlib_source_t> availableSources;
    std::mutex sourcesMutex;
};

#endif // NDI_RECEIVER_H
