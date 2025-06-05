#include "InterruptableThread.hpp"
#include <mutex>
#include <spdlog/spdlog.h>
#include <utility>


namespace jade {

InterruptableThread::InterruptableThread(
    std::function<void()> callback,
    std::chrono::seconds cycleTimeout
) : callback(std::move(callback)),
    cycleTimeout(cycleTimeout) {
    t = std::move(std::thread {
        std::bind(&InterruptableThread::run, this)
    });
}

void InterruptableThread::run() {
    while (running) {
        std::unique_lock l(m);

        spdlog::debug("InterruptableThread: callback invoked");
        callback();
        spdlog::debug("InterruptableThread: callback returned");
        
        control.wait_for(
            l,
            this->cycleTimeout
        );
    }
}

bool InterruptableThread::interrupt() {
    using namespace std::literals;
    // If the lock is held, the update is in progress.
    // `wait_for` releases ownership of the mutex while it's ongoing
    std::unique_lock l(m, std::try_to_lock);
    if (!l.owns_lock()) {
        return false;
    }
    control.notify_one();
    spdlog::info("InterruptableThread: Manual refresh requested");
    std::this_thread::sleep_for(300ms);
    return true;
}

InterruptableThread::~InterruptableThread() {
    kill();
}

}
