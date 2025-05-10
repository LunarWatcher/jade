#include "APIProvider.hpp"

#include "auth/AuthAPI.hpp"
#include "jade/api/settings/SettingsAPI.hpp"
#include <crow.h>


namespace jade {

void APIProvider::init(Server *server) {
    AuthAPI::init(server);
    Settings::init(server);
}


}
