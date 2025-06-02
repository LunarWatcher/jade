#include "jade/util/InterruptableThread.hpp"
#include "util/VFix.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <condition_variable>
#include <spdlog/spdlog.h>

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

TEST_CASE("Validate long interrupts") {
    std::mutex m;
    // Used to simulate a long-running task
    // Funnily enough, this is based around the same system as the thread. I'm sure this isn't bad test practices at all
    jade::tests::VFix<std::condition_variable> interrupt {
        .term = [](auto& v) {
            v.notify_all();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    };
    size_t counter = 0;

    jade::InterruptableThread t(
        [&]() {
            std::unique_lock l(m);
            interrupt->wait_for(l, std::chrono::seconds(60));
            ++counter;
        },
        std::chrono::minutes(60)
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    // Once the initial setup time is cleared, the thread will be stuck on the nested interrupt
    // This means `t.interrupt()` should return false
    REQUIRE_FALSE(t.interrupt());
    REQUIRE(counter == 0);

    // And once the load suddenly disappears, it should work, then cease to work
    interrupt->notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    {
        // The lock should now be released, and counter set to 1. The first cycle has cleared
        std::unique_lock l(m, std::try_to_lock);
        REQUIRE(l.owns_lock());
        REQUIRE(counter == 1);
        l.unlock();
    }

    REQUIRE(t.interrupt());
    INFO("Note: expecting next REQUIRE_FALSE to be for the repeat check");
    REQUIRE_FALSE(t.interrupt());

    std::this_thread::sleep_for(300ms);
    // The test interrupt needs to be manually ntoified here, so the inner thread can release in .kill()
    interrupt->notify_all();
    std::this_thread::sleep_for(300ms);
    REQUIRE(counter == 2);

    // Killing the thread should not affect the counter at this point, since the timeout won't have elapsed, and no
    // further interrupts have been requested
    t.kill();
    std::this_thread::sleep_for(300ms);
    REQUIRE(counter == 2);
}
