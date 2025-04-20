add_library(jadesrc STATIC 
    jade/core/Server.cpp
    jade/core/ContextProvider.cpp

    jade/server/BaseMiddlewares.cpp
    jade/server/UserContextMiddleware.cpp

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
