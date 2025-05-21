#pragma once

#include "crow/middleware.h"
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
 * security-oriented. Specifically, the middleware is designed to have a much easier way to kill sessions, and create
 * new sessions during login. There's also a way to completely kill sessions, which is useful for logging out. 
 *
 * These changes primarily defend against:
 * * OWASP A07[^1], which Crow's built-in session does not handle. Particularly bullets 8 and 9 of the description are
 * ignored
 * * Various pseudorandom attacks. Crow's random engine uses `std::random_device` by default[^2][^3], which is usually
 *      fantastic, but not when dealing with sessions. This session system uses OpenSSL[^4] for a secure random.
 * * Though not yet implemented, token refreshing in this system issues a new token on refresh. Though probably
 *      unnecessary given how long the tokens are, it's just convenient to do.
 *
 * Additionally, the changes themselves wiping cookies means it's easier to avoid retaining bad states on the client
 * side. Not sure if that really matters in practice, but it is a nice benefit. Additionally, due to the middleware
 * being very weirdly designed, this not being obvious makes it very easy to inadvertently fail to invalidate sessions
 * even if you treat the token invalidation as an acceptable problem.
 *
 * As an obligatory note, this application is neither meant to be public-facing, nor is it likely ever going to be a
 * target of any form of attack (and if it is, there's almost certainly other vulnerabilities in crowcpp itself that
 * deal significantly more psychic damage than breached session tokens), but this is hardened for the sake of learning
 * how to harden login and session management.
 *
 * <sub>Honourable mention to V1 of this middleware, used in a now dead project, that was a steaming pile of crap and
 * ignored the A02 violation.</sub>
 *
 * [^1]: https://owasp.org/Top10/A07_2021-Identification_and_Authentication_Failures/
 * [^2]: https://github.com/CrowCpp/Crow/blob/master/include/crow/utility.h#L789
 * [^3]: This happens to fail A02 under failure to ensure cryptographic requirements are met:
 *      https://owasp.org/Top10/A02_2021-Cryptographic_Failures/
 * [^4]: https://docs.openssl.org/3.1/man3/RAND_bytes/
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
    void before_handle(crow::request&, crow::response&, context& ctx, AllContext& globalCtx) {
        crow::CookieParser::context& cookies = globalCtx.template get<crow::CookieParser>();
        auto sessionId = cookies.get_cookie("session");
        if (sessionId.empty()) {
            return;
        }

        auto data = store.getSessionData(sessionId);
        if (data == nullptr) {
            return;
        }

        ctx.data = data;
    }

    template <typename AllContext>
    void after_handle(crow::request&, crow::response&, context& ctx, AllContext& globalCtx) {
        crow::CookieParser::context& cookies = globalCtx.template get<crow::CookieParser>();
        auto currSessToken = cookies.get_cookie("session");

        if (ctx.wantsNewSession) {
            auto newSession = createSession(ctx.data);
            if (!currSessToken.empty()) {
                store.destroy(currSessToken);
            }

            cookies.set_cookie(newSession);
        } else if (ctx.killSession
            // Dead session, session expired, server restarted, failed token fixation attack, etc.
            || (!currSessToken.empty() && !store.getSessionData(currSessToken))
        ) {
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

using MSessionMiddleware =  jade::SessionMiddleware<jade::InMemoryStorage>;

}
