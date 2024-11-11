#ifndef DISCOVERYWORKER_H
#define DISCOVERYWORKER_H

#include <QObject>
#include <Processing.NDI.Lib.h>
#include <QStringList>

class DiscoveryWorker : public QObject
{
    Q_OBJECT
public:
    explicit DiscoveryWorker(NDIlib_find_instance_t ndiFindInstance, QObject *parent = nullptr);
    ~DiscoveryWorker();

signals:
    void sourcesDiscovered(const QStringList &sources);

public slots:
    void process();

private:
    NDIlib_find_instance_t ndiFindInstance;
};

#endif // DISCOVERYWORKER_H
