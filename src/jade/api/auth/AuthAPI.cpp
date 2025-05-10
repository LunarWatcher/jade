#include "AuthAPI.hpp"

#include <crow.h>

#include "crow/common.h"
#include "crow/middlewares/session.h"
#include "jade/api/auth/AuthAPI.hpp"
#include "jade/core/Macros.hpp"
#include "jade/core/Server.hpp"

#include "jade/data/AuthRequest.hpp"
#include "jade/data/User.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/security/Hash.hpp"
#include "jade/util/ResponseUtil.hpp"
#include "spdlog/spdlog.h"

namespace jade {

void AuthAPI::init(Server *server) {
    CROW_ROUTE(server->app, "/api/auth/login")
        .methods(crow::HTTPMethod::POST)
        (JADE_CALLBACK_BINDING(AuthAPI::login));

    CROW_ROUTE(server->app, "/api/auth/signup")
        .methods(crow::HTTPMethod::POST)
        (JADE_CALLBACK_BINDING(AuthAPI::signup));

    // Pseudo-catchall, leaving this for future reference since blueprints have been broken for around 1.5 years, and
    // I'm probably not gonna shoehorn them in after the fact:
    // * https://github.com/CrowCpp/Crow/issues/714
    // I'm not sure if this has to be defined absolutely last or not. Also note that this is invoked for what would've
    // been a 405, which isn't optimal.
    //CROW_ROUTE(server->app, "/api/auth/<path>")
        //([](const std::string& path) {
            //return "Catchall?";
        //});
}

void AuthAPI::login(Server *server, crow::request &req, crow::response &res) {
    auto& userCtx = (*server)->get_context<MSessionMiddleware>(req);

    if (userCtx.data && userCtx.data->user) {
        res.code = 200;
        res.body = R"({"message": "already logged in"})";
        return;
    }


    LoginRequest data = nlohmann::json::parse(req.body);

    server->pool->acquire<void>([&](auto& conn) {
        pqxx::work tx(conn);
        auto row = tx.query01<long long, std::string, std::string, std::string, long long, bool>(
            "SELECT UserID, Username, Password, Salt, Version, IsAdmin FROM Jade.Users WHERE Username = $1",
            pqxx::params{
                data.username
            }
        );

        if (!row.has_value()) {
            res = JSONResponse {
                nlohmann::json {
                    {"message", "User does not exist, or the password is incorrect"},
                }
            };
            res.code = 401;
            return;
        }

        auto inputPassword = Hash::hash(data.password, std::get<3>(*row), std::get<4>(*row));
        if (inputPassword.hash != std::get<2>(*row)) {
            spdlog::info("{} provided a bad password", req.remote_ip_address);
            res = JSONResponse {
                nlohmann::json {
                    {"message", "User does not exist, or the password is incorrect"},
                }
            };
            res.code = 401;
            return;
        }

        // Login succeeded
        // TODO: handle password upgrades

        userCtx.newSession();
        User u {
            std::get<0>(*row),
            std::get<1>(*row),
            std::get<5>(*row),
        };
        res = JSONResponse {
            nlohmann::json {{"user", u},}
        };
        userCtx.data->user = u;

    });

    res.end();
}

void AuthAPI::signup(Server *server, crow::request &req, crow::response &res) {
    auto& userCtx = (*server)->get_context<MSessionMiddleware>(req);

    if (userCtx.data && userCtx.data->user) {
        res = JSONResponse{
            R"({"message": "you already have an account, wtf are you doing?"})"
        };
        res.code = 400;
        return;
    }

    SignupRequest data = nlohmann::json::parse(req.body);


    server->pool->acquire<void>([&](auto& conn) {
        pqxx::work tx(conn);
        auto exists = tx.query1<bool>(
            "SELECT EXISTS(SELECT 1 FROM Jade.Users WHERE Username = $1)", 
            pqxx::params {data.username}
        );
        auto hasExistingUser = tx.query1<long long>("SELECT COUNT(*) FROM Users");
        if (std::get<0>(exists)) {
            res = JSONResponse {
                nlohmann::json{
                    {"message", "Username taken"},
                }
            };
            res.code = 400;
            return;
        }

        auto password = Hash::hash(data.password, std::nullopt);
        tx.exec(R"(
        INSERT INTO Jade.Users (Username, Password, Salt, Version, IsAdmin)
        VALUES ($1, $2, $3, $4, $5);
        )", pqxx::params {
            data.username,
            password.hash,
            password.salt,
            password.version,
            // Only the first user is auto-granted admin, every other user needs to be granted it manually.
            std::get<0>(hasExistingUser) == 0
        });
        tx.commit();

        res = JSONResponse {
            nlohmann::json {
                {"message", "ok"}
            }
        };
    });

    res.end();
}

}
