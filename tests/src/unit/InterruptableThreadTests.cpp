#include "jade/util/InterruptableThread.hpp"
#include "util/DynamicWorkloadIntThread.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <spdlog/spdlog.h>
#include <thread>

using namespace std::literals;

TEST_CASE("Validate basic interrupts") {
    std::mutex m;
    size_t counter = 0;

    jade::InterruptableThread t(
        [&]() {
            std::unique_lock l(m);
            ++counter;
        },
        std::chrono::seconds(60)
    );
    // Give the thread time to do its initial pass
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // More or less immediately after 
    std::unique_lock l(m);
    REQUIRE(counter == 1);
    l.unlock();


    REQUIRE(t.interrupt());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    l.lock();
    REQUIRE(counter == 2);
    l.unlock();

    Catch::Timer timer;
    timer.start();
    t.kill();
    REQUIRE(timer.getElapsedSeconds() < 0.5);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    REQUIRE(counter == 2);
}

