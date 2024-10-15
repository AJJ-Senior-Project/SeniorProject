#ifndef NDI_SENDER_H
#define NDI_SENDER_H

#include <cstddef>  // For NULL definition, if necessary
#include <iostream> // Include other necessary headers
#include <Processing.NDI.Lib.h>
#include <QObject>

class NDISender : public QObject{
    Q_OBJECT
public:
    NDISender();
    ~NDISender();
    void initializeSender();
    void sendFrame(const QImage &frame);
private:
    NDIlib_send_instance_t ndi_send_instance;
};



#endif // NDI_SENDER_H
