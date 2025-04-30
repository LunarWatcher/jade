#pragma once

#include "crow/middlewares/cookie_parser.h"
#include "jade/data/User.hpp"
#include "jade/meta/Common.hpp"
#include "jade/security/Random.hpp"
#include <chrono>
#include <concepts>
#include <shared_mutex>

#include <crow.h>

namespace jade {

struct SessionData {
    std::optional<User> user;

    std::chrono::time_point<Clock> validUntil; 
};

struct Storage {
    virtual void create(const std::string& key, std::shared_ptr<SessionData> init) = 0;
    virtual void create(const std::string& key, const SessionData& init) {
        create(key, std::make_shared<SessionData>(init));
    }
    virtual void destroy(const std::string& key) = 0;
    virtual void refresh(const std::string& oldKey, const std::string& newKey) = 0;
    virtual std::shared_ptr<SessionData> getSessionData(const std::string& key) = 0;
};

/**
 * Session middleware inspired by Crow's SessionMiddleware, but written from scratch to be more portable and more
 * security-oriented.
 */
template <typename T>
requires CheckType<T, Storage>
class SessionMiddleware {
private:
    std::chrono::seconds inactivityInterval;
    T store;
public:
    struct context {
        std::shared_ptr<SessionData> data;

        /**
         * Creates a new session; must be called prior to modifying data, so the data pointer doesn't point to an old
         * session
         */
        void newSession() {
            data = std::make_shared<SessionData>();
            wantsNewSession = true;
        }

        void kill() {
            data = nullptr;
            killSession = true;
        }
    private:
        bool wantsNewSession = false;
        bool killSession = false;

        friend class SessionMiddleware;
    };

    SessionMiddleware() requires std::default_initializable<T> { }

    /**
     * Alternate constructor, must be used if Storage isn't default-initialisable
     */
    SessionMiddleware(T s) : store(s) {}

    template <typename AllContext>
    void before_handle(crow::request& req, crow::response&, context& ctx, AllContext& globalCtx) {
        crow::CookieParser::context& cookies = globalCtx.template get<crow::CookieParser>();
        auto sessionId = cookies.get_cookie("session");
        if (sessionId == "") {
            return;
        }

        auto data = store.getSessionData(sessionId);
        if (data == nullptr) {
            return;
        }

        ctx.data = data;
    }

    template <typename AllContext>
    void after_handle(crow::request& req, crow::response& res, context& ctx, AllContext& globalCtx) {
        crow::CookieParser::context& cookies = globalCtx.template get<crow::CookieParser>();
        auto currSessToken = cookies.get_cookie("session");
        std::cout << currSessToken << std::endl;

        if (ctx.wantsNewSession) {
            auto newSession = createSession(ctx.data);
            if (currSessToken != "") {
                store.destroy(currSessToken);
            }

            cookies.set_cookie(newSession);
        } else if (ctx.killSession
            // Dead session, session expired, server restarted, failed token fixation attack, etc.
            || (currSessToken != "" && !store.getSessionData(currSessToken))
        ) {
            spdlog::warn("Killing session");
            // Cookie invalid or session requested killed; destroy cookie, and kill the session if it exists
            store.destroy(currSessToken);
            cookies.set_cookie(
                crow::CookieParser::Cookie {
                    "session", "invalid"
                }
                    // TODO: This needs to line up with createSession(); maybe find a way to unify the two?
                    .httponly()
                    .expires(std::tm {0})
                    .path("/")
            );

        }

    }

    crow::CookieParser::Cookie createSession(std::shared_ptr<SessionData> sourceData) {
        if (sourceData == nullptr) {
            // No supplied data means blank new session. Create a blank data pointer
            sourceData = std::make_shared<SessionData>();
        }
        auto token = generateToken();

        store.create(token, sourceData);
        spdlog::info("New session established for user {}", sourceData->user.has_value() ? sourceData->user->username : "<no user>");

        auto maxAge = getMaxAge();
        auto end = Clock::now() + maxAge;

        sourceData->validUntil = end;

        // TODO: set the domain. Requires a domain to be set in config or something
        // TODO: figure out `.secure()`
        return crow::CookieParser::Cookie {
            "session", token
        }
            .httponly()
            .path("/")
            .max_age( // https://mrcoles.com/blog/cookies-max-age-vs-expires/
                std::chrono::duration_cast<std::chrono::seconds>(
                    maxAge
                ).count()
            );
    }

    auto getMaxAge() {
        return std::chrono::days(30);
    }
};


struct InMemoryStorage : public Storage {
    std::shared_mutex m;
    std::map<std::string, std::shared_ptr<SessionData>> data;

    void create(const std::string& key, std::shared_ptr<SessionData> init) override;
    void destroy(const std::string& key) override;
    void refresh(const std::string& oldKey, const std::string& newKey) override;

    std::shared_ptr<SessionData> getSessionData(const std::string& key) override;

};

}
