#pragma once

#include "crow/mustache.h"
#include <crow.h>
#include <crow/json.h>
#include <stdexcept>
#include <vector>

namespace jade {
class Server;

namespace ContextProvider {

/**
 * Defines what additional (but still standard) data objects to add to the mustache context.
 *
 * Note for developers: Due to how the context scope is set, each enum value must be 2^n, so they can be binary `or`ed
 * to allow multiple scopes. This theoretically limits the number of scopes to 64, which is far more than what's
 * reasonable to include in code.
 */
enum ContextScope {
    USER = 1,
    LIBRARIES = 2,
};

struct PageContext {
    std::string pageTitle;
    std::string pageDescription;
    std::string pageFile;
    std::vector<crow::json::wvalue> pageScripts = {};
    std::vector<crow::json::wvalue> pageCSS = {};

    bool includeSidebar = false;
    bool hideFooter = false;

    /**
     * DO NOT FILL! Place all fields before this. This field will be auto-populated.
     */
    crow::mustache::context generated;
    bool inited = false;

    const crow::mustache::context& getContext() {
        if (!inited) {
            if (pageFile.empty()) {
                throw std::runtime_error("Programmer error: pageFile is undefined for pagetitle=" + pageTitle);
            }
            inited = true;
            generated["Server"] = {
                {"Version", JADE_VERSION},
            };
            generated["Page"] = crow::json::wvalue {
                {"Title", pageTitle},
                {"Description", pageDescription},
                {"Scripts", pageScripts},
                {"CSS", pageCSS}
            };
            generated["Content"] = [&](std::string&) { \
                return std::string("{{>" + pageFile + "}}"); \
            };

            generated["Special"] = {
                {"Sidebar", this->includeSidebar},
                {"HideFooter", this->hideFooter}
            };
        }

        return generated;
    }
};

/**
 * \param contextScope      &-ed values from ContextScope. A base page context is always built.
 * \param serv              The Server instance: only used if contextScope != 0, and must be non-null if contextScope !=
 *                          0. May be null if contextScope is 0
 */
extern crow::mustache::context buildBaseContext(
    int contextScope,
    crow::request& req,
    PageContext& baseCtx,
    Server* serv
);

/**
 * Utility function for loading body.mustache (and in release mode, caching body.mustache)
 */
inline crow::mustache::template_t getBaseTemplate() {
#ifndef JADE_DEBUG
    // Static is disabled in debug mode to allow easier template reloads.
    // This does not affect sub-partials, but avoids having to kill the server for changes to body.mustache to be
    // reflected. 
    //
    // Avoiding templates in prod is done to avoid unnecessary IO. Not sure how much it matters.
    static
#endif
    crow::mustache::template_t body = crow::mustache::load("partials/body.mustache");

    return body;
}

namespace _detail {

extern void buildUserContext(
    crow::mustache::context& ctx,
    crow::request& req,
    Server* serv
);

extern void buildLibraryContext(
    crow::mustache::context& ctx,
    crow::request& req,
    Server* serv
);

}

}
}
