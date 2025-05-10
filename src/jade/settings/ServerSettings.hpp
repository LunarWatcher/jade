#pragma once

namespace jade {

/**
 * Contains various non-basic server settings. Must not be confused with ServerConfig, which is the low-level server
 * config required for the server to start at all.
 *
 * This struct defines various shared server settings that can, unlike the config, can be changed at runtime by the
 * admins or in very rare cases, by the system.
 *
 * For user-controlled per-user settings, see UserSettings.
 */
struct ServerSettings {
    bool registrationEnabled;

};

}
