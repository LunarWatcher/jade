add_library(jadesrc STATIC 
    jade/api/APIProvider.cpp
    jade/api/auth/AuthAPI.cpp
    jade/api/books/BookAPI.cpp
    jade/api/settings/SettingsAPI.cpp

    jade/cli/CLIMigrations.cpp

    jade/conf/ServerConfig.cpp

    jade/core/ContextProvider.cpp
    jade/core/Server.cpp

    
    jade/data/AuthRequest.cpp
    jade/data/BookRequests.cpp
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

    jade/util/InterruptableThread.cpp
    jade/util/ResponseUtil.cpp

    jade/web/WebProvider.cpp
)

list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

add_executable(jade jade/Main.cpp)

target_link_libraries(jadesrc
PUBLIC
    stc
    cpr::cpr
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Crow::Crow
    OpenSSL::Crypto
    CLI11::CLI11
    libpqxx::pqxx
)

target_include_directories(jadesrc PUBLIC ./)

target_link_libraries(jade jadesrc)

add_custom_target(webcontent
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/www ${CMAKE_BINARY_DIR}/www
    COMMENT "Symlink debug symlink repo"
)
add_dependencies(jadesrc webcontent)

install(TARGETS jade
    DESTINATION bin)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/lib
    DESTINATION .
    FILES_MATCHING
        PATTERN *.so*
        PATTERN *.a EXCLUDE
)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/www
    DESTINATION .
)

if (JADE_LINT)
    find_program(
        CLANGTIDY
        NAMES "clang-tidy" "clang-tidy.exe"
        REQUIRED
    )

    set (CLANG_TIDY_COMMAND "${CLANGTIDY}" "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/../.clang-tidy")
    if (LINT_WARNINGS_ARE_ERRORS)
        list (APPEND CLANG_TIDY_COMMAND  -warnings-as-errors=*)
    endif()

    if (WIN32)
        list (APPEND CLANG_TIDY_COMMAND --extra-arg=/EHsc)
    endif()

    message(STATUS "Using CLANG_TIDY_COMMAND=${CLANG_TIDY_COMMAND}")
    set_target_properties(jadesrc PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

# vim:ft=cmake
