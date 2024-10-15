#ifndef NDISENDER_H
#define NDISENDER_H

#include <Processing.NDI.Lib.h>
#include <QObject>
#include <QImage>  // Include QImage here

class NDISender : public QObject {
    Q_OBJECT
public:
    NDISender();
    ~NDISender();

    void initializeSender();
    void sendFrame(const QImage &frame);  // Declaration of sendFrame

private:
    NDIlib_send_instance_t ndi_send_instance;
};

#endif // NDISENDER_H
