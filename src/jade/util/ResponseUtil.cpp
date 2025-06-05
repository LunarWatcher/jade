#include "ResponseUtil.hpp"
#include <string>

#include <crow.h>

namespace jade {

std::string JSONResponse::dump() const {
    return body;
}

}
