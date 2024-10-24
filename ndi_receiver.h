#ifndef NDI_RECEIVER_H
#define NDI_RECEIVER_H

#include <cstddef>  // For NULL definition, if necessary
#include <iostream>
#include <Processing.NDI.Lib.h>
#include <string>
#include <vector>
#include <iostream>

class NDIReceiver
{
public:
    NDIReceiver();
    ~NDIReceiver();

    bool initializeReceiver(); // Initialize NDI receiver
    void discoverSources(); // Discover NDI sources
    bool selectSource(const std::string& sourceName); // Select a specific NDI source
    void receiveVideoAndMetadata(); // Receive video and metadata from selected sources
    void terminateReceiver(); // Terminate the receiver
    void printAvailableSources(); // Print discovered sources

private:
    NDIlib_find_instance_t ndiFindInstance; // NDI finder for sources
    std::vector<NDIlib_source_t> availableSources; // List of available sources
    NDIlib_recv_instance_t ndiReceiverInstance; // NDI receiver instance
};

#endif // NDI_RECEIVER_H
