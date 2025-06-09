#include "jade/data/AuthRequest.hpp"
#include "jade/util/Constants.hpp"
#include "util/ServerWrapper.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/api.h>
#include <cpr/body.h>
#include <cpr/cprtypes.h>
#include <cpr/redirect.h>

TEST_CASE("Basic signup", "[Auth]") {
    jade::tests::ServerWrapper s(false);

    SECTION("Verify that the first user is an admin") {
        auto [username, password] = s.getAdminCreds();
        auto r = cpr::Post(
            s / "api/auth/signup",
            cpr::Body { rfl::json::write(jade::SignupRequest {
                .username = username,
                .password = password
            }) },
            cpr::Header {
                { "Content-Type", jade::ContentType::json }
            }
        );

        REQUIRE(r.status_code == 200);

        auto login = cpr::Post(
            s / "api/auth/login",
            cpr::Body { rfl::json::write(jade::SignupRequest {
                .username = username,
                .password = password
            }) },
            cpr::Header {
                { "Content-Type", jade::ContentType::json }
            }
        );
        INFO(login.text);
        REQUIRE(login.status_code == 200);
        auto resp = rfl::json::read<jade::LoginResponse>(login.text);
        if (!resp) {
            FAIL(resp.error().what());
        }
        auto& [user] = resp.value();
        REQUIRE(user.userId == 1);
        REQUIRE(user.username == "admin");
        REQUIRE(user.isAdmin);
    }
    SECTION("Verify that duplicate usernames cannot be created") {
        auto [username, password] = s.getAdminCreds();
        std::vector<int> responses = { 200, 400 };
        for (int i = 0; i < 2; ++i) {
            auto r = cpr::Post(
                s / "api/auth/signup",
                cpr::Body { rfl::json::write(jade::SignupRequest {
                    .username = username,
                    .password = password
                }) },
                cpr::Header {
                    { "Content-Type", jade::ContentType::json }
                }
            );
            INFO(r.text);
            REQUIRE(r.status_code == responses.at(i));
        }
    }
}

TEST_CASE("Verify that bad username and bad password are indistinguishable", "[Auth][Security]") {
    jade::tests::ServerWrapper s;
    auto [username, password] = s.getAdminCreds();
    std::vector<std::string> responses;
    std::vector<std::pair<std::string, std::string>> creds = {
        {username, password + "1"},
        {username + "1", password}
    };
    for (int i = 0; i < 2; ++i) {
        auto& [username, password] = creds.at(i);
        auto r = cpr::Post(
            s / "api/auth/login",
            cpr::Body { rfl::json::write(jade::SignupRequest {
                .username = username,
                .password = password
            }) },
            cpr::Header {
                { "Content-Type", jade::ContentType::json }
            }
        );
        INFO(r.text);
        INFO(username);
        INFO(password);
        REQUIRE(r.status_code == 401);
        responses.push_back(r.text);
    }

    REQUIRE(responses.at(0) == responses.at(1));
    REQUIRE_FALSE(responses.at(0).empty());
}

TEST_CASE("Verify that cookie injection doesn't work", "[Auth][Security]") {
    jade::tests::ServerWrapper s;
    auto [username, password] = s.getAdminCreds();

    auto r = cpr::Post(
        s / "api/auth/login",
        cpr::Body { rfl::json::write(jade::SignupRequest {
            .username = username,
            .password = password
        }) },
        cpr::Header {
            { "Content-Type", jade::ContentType::json }
        },
        cpr::Cookies {
            {"session", "iliketrains"}
        }
    );
    INFO(r.text);
    REQUIRE(r.status_code == 200);
    bool foundSession = false;
    for (auto& cookie : r.cookies) {
        if (cookie.GetName() == "session") {
            REQUIRE(cookie.GetValue() != "iliketrains");
            foundSession = true;
        }
    }

    REQUIRE(foundSession);

    SECTION("Check that reusing the bogus token didn't secretly work") {
        auto r = cpr::Get(
            cpr::Url {s.getUrl()},
            cpr::Cookies {
                {"session", "iliketrains"}
            },
            cpr::Redirect{false}
        );
        REQUIRE(r.status_code == 307);
        for (auto& cookie : r.cookies) {
            if (cookie.GetName() == "session") {
                REQUIRE(cookie.GetExpires() == std::chrono::system_clock::time_point{});
            }
        }
    }
    SECTION("Verify that the provided cookies do work") {
        auto w = cpr::Get(
            cpr::Url {s.getUrl()},
            r.cookies,
            cpr::Redirect{false}
        );
        REQUIRE(w.status_code == 200);
    }
}

