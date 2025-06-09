#pragma once

#include "jade/util/InterruptableThread.hpp"
#include <chrono>
#include <condition_variable>
#include <spdlog/spdlog.h>

namespace jade::tests {
using namespace std::literals;

/**
 * Utility class wrapping InterruptableThread, and a mutex and an interrupt that can be used to dynamically control how
 * long it takes to execute. Largely a utility to avoid unnecessary sleeps.
 */
struct DynamicWorkloadIntThread {
    std::function<void()> threadWorkload;
    std::mutex metaMutex;
    std::condition_variable metaInterrupt;
    std::condition_variable mainSync;

    InterruptableThread t;

    DynamicWorkloadIntThread(
        std::function<void()> threadWorkload,
        std::chrono::minutes timeout = 60min
    ) 
        : threadWorkload(threadWorkload), // NOLINT
        t(InterruptableThread {
            [this]() {
                std::unique_lock l(metaMutex);
                mainSync.notify_all();
                metaInterrupt.wait_for(l, std::chrono::seconds(3));

                this->threadWorkload();
                mainSync.notify_all();
            },
            timeout
        }) {}

    ~DynamicWorkloadIntThread() {
        spdlog::debug("Destroying DynamicWorkloadIntThread");
        kill();
    }

    void doMetaInterrupt() {
        metaInterrupt.notify_all();
    }

    void kill() {
        metaInterrupt.notify_all();
        t.kill();
    }

    InterruptableThread* operator->() {
        return &t;
    }

};

}
