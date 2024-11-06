#ifndef NDI_RECEIVER_H
#define NDI_RECEIVER_H

#include <cstddef>  // For NULL definition, if necessary
#include <iostream>
#include <Processing.NDI.Lib.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

class NDIReceiver
{
public:
    NDIReceiver();
    ~NDIReceiver();

    bool initializeReceiver(); // Initialize NDI receiver
    void discoverSources(); // Discover NDI sources
    bool selectSource(const std::string& sourceName); // Select a specific NDI source
    void startReceiving(); // Start receiving video and metadata for all sources
    void terminateReceiver(); // Terminate the receiver and all threads
    void printAvailableSources(); // Print discovered sources


    void receiveVideoAndMetadata(const NDIlib_source_t& source); // Receive video and metadata for a specific source
private:
    NDIlib_find_instance_t ndiFindInstance; // NDI finder for sources
    std::vector<NDIlib_source_t> availableSources; // List of available sources
    std::unordered_map<std::string, std::thread> receiverThreads; // Threads for each source
    std::unordered_map<std::string, NDIlib_recv_instance_t> ndiReceiverInstances; // Receiver instances for each thread
    std::mutex sourcesMutex; // Mutex to protect shared data
    std::atomic<bool> receiving; // Flag to control receiving state
};

#endif // NDI_RECEIVER_H
