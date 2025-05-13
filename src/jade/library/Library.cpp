#include "Library.hpp"
#include "jade/health/HealthCheck.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <iterator>

#include <stc/Environment.hpp>
#include <stc/StringUtil.hpp>

#include <jade/core/Server.hpp>

namespace jade {

Library::Library(Server* s, pqxx::connection& conn) : serv(s) {
    pqxx::work w(conn);
    auto libraries = w.query<int64_t, std::string>("SELECT LibraryID, Location FROM Jade.Libraries");

    std::transform(
        libraries.begin(), libraries.end(), 
        std::inserter(sources, sources.begin()),
        [](const auto& row) -> std::pair<int64_t, Source> {
            return {
                std::get<0>(row),
                Source {
                    std::get<1>(row)
                }
            };
        }
    );

    if (sources.size() == 0) {
        spdlog::warn("No active libraries");
    }

}

void Library::scan() {
    ThreadCanary canary(this);
    while (true) {
        spdlog::debug("Index starting");
        for (auto& [id, source] : sources) {
            auto& dir = source.dir;
            // Under ext4, adding or updating a file results in last_write_time being updated.
            if (auto newUpdate = std::filesystem::last_write_time(dir);
                newUpdate > source.lastUpdate)
            {
                source.lastUpdate = newUpdate;

                reindexLibrary(id, dir);
            } else {
                spdlog::debug("{} is up to date", source.dir.string());
            }
        }

        if (sources.size() == 0) {
            spdlog::info("Scanner: No libraries exist to scan");
        } else {
            spdlog::debug("Index complete");
        }
        std::this_thread::sleep_for(std::chrono::minutes(60));
    }
}

void Library::initScanProcess() {
    runner = std::move(std::thread(
        std::bind(&Library::scan, this)
    ));
}

bool Library::createLibrary(Server* serv, const std::string& location) {
    return serv->pool->acquire<bool>([&](auto& conn) {
        pqxx::work w(conn);

        auto exists = w.query1<bool>(
            "SELECT EXISTS(SELECT 1 FROM Jade.Libraries WHERE Location = $1)", 
            pqxx::params {location}
        );

        if (std::get<0>(exists)) {
            return false;
        }

        auto res = w.exec(R"(INSERT INTO Jade.Libraries (LibraryID, Location) VALUES (DEFAULT, $1) RETURNING LibraryID)",
            pqxx::params {
                location
            }
        ).one_row();
        auto id = res.at(0).as<int64_t>();
        spdlog::info("LibraryID {} (loc: {}) created", id, location);

        w.commit();
        std::unique_lock l(m);
        sources[id] = Source {
            location
        };
        return true;
    });
}

std::vector<health::HealthResult> Library::checkHealth() {
    std::vector<health::HealthResult> out;

    {
        std::vector<health::HealthCheck> sourceChecks;

        for (auto& [id, source] : this->sources) {
            auto isValid = source.isValid();
            sourceChecks.push_back({
                isValid,
                source.dir.string(),
                isValid ? "" : "Path does not exist, or is not a directory"
            });
        }

        sourceChecks.push_back({
            !runnerDead,
            "Library scanner thread",
            runnerDead ? "Defunct" : "Running"
        });

        out.push_back({
            "Library sources",
            sourceChecks
        });

    }

    return out;
}

void Library::reindexLibrary(int64_t sourceId, const std::filesystem::path& dir) {
    serv->pool->acquire<void>([&](auto& conn) {
        pqxx::work w(conn);
        for (auto& file : std::filesystem::directory_iterator(dir)) {
            auto path = std::filesystem::relative(
                file.path(),
                dir
            );
            auto ext = path.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](const auto& ch) {
                return std::tolower(ch);
            });
            
            if (std::find(SUPPORTED_EXTENSIONS.begin(), SUPPORTED_EXTENSIONS.end(), ext) == SUPPORTED_EXTENSIONS.end()) {
                spdlog::debug("{} is not supported ({} is not a valid extension)", path.string(), ext);
                continue;
            }

            if (std::get<0>(
                w.query1<bool>(
                    "SELECT EXISTS(SELECT 1 FROM Jade.Books WHERE FileName = $1)", 
                    pqxx::params {
                        path.string()
                    }
                )
            ) == false) {
                spdlog::info("New book detected in {}: {}", dir.string(), path.string());

                auto strPath = (dir/path).string();
                int code;
                auto output = stc::syscommand(
                    std::vector {
                        metaCommand.c_str(),
                        strPath.c_str()
                    }
                );

                std::string title;
                if (code != 0) {
                    auto lines = stc::string::split(output, '\n');

                    for (auto& line : lines) {
                        if (line.starts_with("Title")) {
                            // + 2 is required to exclude the colon, and exclude the space immediately after it
                            title = line.substr(line.find(':') + 2);
                            break;
                        }
                    }
                }

                auto res = w.exec(
                    R"(INSERT INTO Jade.Books (FileName, LibraryID, Title) VALUES ($1, $2, $3) RETURNING BookID;)",
                    
                    pqxx::params {
                        path.string(),
                        sourceId,
                        title
                    }
                );

                auto insertedId = res.at(0).as<int64_t>();

                generateThumbnail(std::get<0>(insertedId), dir / path);
            }

        }

        // TODO: prepared statements don't seem to work with the stream API. Need to report this as a bug at some point,
        // but until then, this is an extremely rare exception where I'll do hard-coded values. It's just an int, so no
        // injection risk.
        for (auto [bookId, filename] : w.stream<int64_t, std::string_view>(
            "SELECT BookID, FileName FROM Jade.Books WHERE LibraryID = " + std::to_string(sourceId)
            //pqxx::params {
                //sourceId
            //}
        )) {
            if (!std::filesystem::exists(dir / filename)) {
                w.exec("DELETE FROM Jade.Books WHERE BookID = $1", pqxx::params {
                    bookId
                });
                spdlog::info("Book {} no longer exists and has been removed", bookId);
            }
        }

        w.commit();
    });
}

void Library::generateThumbnail(int64_t id, const std::filesystem::path& file) {
    auto output = serv->getConfig().thumbnailCacheDir / (std::to_string(id) + ".png");

    int code;
    stc::syscommand(std::vector {
        metaCommand.c_str(), file.string().c_str(),
        ("--get-cover=" + output.string()).c_str()
    }, &code);
    if (code != 0) {
        spdlog::error("Failed to generate thumbnail for {} (error code {})", file.string(), code);
    } else {
        spdlog::debug("Generated thumbnail for {}", file.string());
    }

}

}
