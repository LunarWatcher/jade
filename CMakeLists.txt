cmake_minimum_required(VERSION 3.28)
project(jade)

option (JADE_SANITISE "Whether or not to enable -fsanitize=undefined" OFF)
option (JADE_LINT "Whether or not to enable clang-tidy" OFF)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list (PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

include(FetchContent)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CURL_ZLIB ON CACHE STRING "" FORCE)
set(SPDLOG_USE_STD_FORMAT ON CACHE STRING "" FORCE)

# TODO: Check if newer versions of crow axed CROW_FEATURES
set(CROW_FEATURES ssl compression CACHE STRING "" FORCE)
set(CROW_INSTALL OFF CACHE STRING "" FORCE)
set(CROW_ENABLE_COMPRESSION ON CACHE STRING "" FORCE)


FetchContent_Declare(pqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx
    GIT_TAG 8d48b64b36158a50dc6ed33001c793652065b833
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(crowcpp
    GIT_REPOSITORY https://github.com/CrowCpp/Crow
    #GIT_TAG v1.2.1.2
    EXCLUDE_FROM_ALL
)


set(ZLIB_COMPAT ON CACHE INTERNAL "" FORCE)
set(ZLIB_ENABLE_TESTS OFF CACHE INTERNAL "" FORCE)

FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.2
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(nlohmann
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.12.0
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.15.2
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(stc
    GIT_REPOSITORY https://github.com/LunarWatcher/stc
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG main
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(crowcpp
    pqxx
    nlohmann
    spdlog
    cpr
    stc
    cli11
)

find_package(OpenSSL REQUIRED)

add_definitions(-DCROW_ENFORCE_WS_SPEC=1)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DJADE_DEBUG=1)
endif()

if (JADE_SANITISE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

# Set up versioning {{{
find_program(GIT git)
if (GIT)
    execute_process(COMMAND ${GIT} describe --tags --always
        OUTPUT_VARIABLE JADE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Detected version: ${JADE_VERSION}")
else()
    message(WARNING "Git not found. Defaulting to unknown version. This will not have an effect on the program beyond a missing version")
    set(JADE_VERSION "VERSION UNKNOWN. Who doesn't have Git? It's the 2020s. Install it already")
endif()
add_definitions(-DCROW_STATIC_DIRECTORY="www/static/")
add_definitions(-DJADE_VERSION="${JADE_VERSION}")
# }}}

add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)

add_custom_target(run
    COMMAND jade
    DEPENDS jade
    COMMENT "Run jade")
add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Test jade (backend)")
add_custom_target(test-frontend
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    COMMAND SERVER_LOC=$<TARGET_FILE:jade> python3 -m unittest -b
    DEPENDS jade
    COMMENT "Test jade (frontend + integration)"
)
add_custom_target(test-all
    DEPENDS test test-frontend
    COMMENT "Run backend and frontend tests")
# vim:ft=cmake
