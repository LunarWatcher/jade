#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>

namespace jade {

class InterruptableThread {
private:
    std::mutex m;
    std::condition_variable control;
    bool running = true;
    
    std::thread t;
    std::function<void()> callback;
    std::chrono::seconds cycleTimeout;

    void run();
public:
    InterruptableThread() = default;
    InterruptableThread(
        std::function<void()> callback,
        std::chrono::seconds cycleTimeout
    );
    InterruptableThread(const InterruptableThread&) = delete;

    ~InterruptableThread();

    bool interrupt();
    void kill() { 
        if (!running) {
            return;
        }
        std::unique_lock l(m);
        running = false; 
        l.unlock();
        control.notify_all();
        t.join(); 
    }
};

}
