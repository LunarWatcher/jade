#include "APIProvider.hpp"

#include "auth/AuthAPI.hpp"
#include <crow.h>


namespace jade {

void APIProvider::init(Server *server) {
    AuthAPI::init(server);
}


}
