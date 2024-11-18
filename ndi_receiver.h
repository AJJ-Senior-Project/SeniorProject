#ifndef NDI_RECEIVER_H
#define NDI_RECEIVER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QStringList>
#include <QThread>
#include <mutex>
#include <map>

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
    void stopReceiving();
    void terminateReceiver();

signals:
    void sourcesDiscovered(const QStringList &sources);
    void frameReceived(const QString &sourceName, const QImage &frame);

    void stopReceivingSignal();  // Signal to stop the receiver workers

private:
    NDIlib_find_instance_t ndiFindInstance;
    std::map<QString, NDIlib_recv_instance_t> ndiReceiverInstances;
    std::map<QString, QThread*> receiverThreads;
    std::map<QString, ReceiverWorker*> receiverWorkers;
    QThread *discoveryThread;
    DiscoveryWorker *discoveryWorker;
    std::mutex sourcesMutex;
};

#endif // NDI_RECEIVER_H
