add_library(jadesrc STATIC 
    jade/api/APIProvider.cpp
    jade/api/auth/AuthAPI.cpp
    jade/api/books/BookAPI.cpp
    jade/api/settings/SettingsAPI.cpp

    jade/conf/ServerConfig.cpp

    jade/core/ContextProvider.cpp
    jade/core/Server.cpp

    
    jade/data/AuthRequest.cpp
    jade/data/SettingsRequests.cpp
    jade/data/User.cpp
    jade/data/db/Book.cpp

    jade/db/ConnectionPool.cpp
    jade/db/Migration.cpp
    jade/db/security/Hash.cpp

    jade/library/Library.cpp

    jade/server/BaseMiddlewares.cpp
    jade/server/CacheHelper.cpp
    jade/server/SessionMiddleware.cpp

    jade/util/ResponseUtil.cpp

    jade/web/WebProvider.cpp
)
add_executable(jade jade/Main.cpp)

target_link_libraries(jadesrc
PUBLIC
    stc
    zlib
    cpr::cpr
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Crow::Crow
    OpenSSL::Crypto
    CLI11::CLI11
    pqxx
)

target_include_directories(jadesrc PUBLIC ./)

target_link_libraries(jade jadesrc)

add_custom_target(webcontent
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/www ${CMAKE_BINARY_DIR}/www
    COMMENT "Symlink debug symlink repo"
)


add_dependencies(jadesrc webcontent)

# vim:ft=cmake
