cmake_minimum_required(VERSION 3.28)

include(FetchContent)
FetchContent_Declare(conan
    GIT_REPOSITORY https://github.com/conan-io/cmake-conan
    GIT_TAG develop2
)
FetchContent_MakeAvailable(conan)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${conan_SOURCE_DIR}/conan_provider.cmake)

project(jade)

option (JADE_SANITISE "Whether or not to enable -fsanitize=undefined" OFF)
option (JADE_LINT "Whether or not to enable clang-tidy" OFF)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CONAN_INSTALL_ARGS "--build=missing" "-druntime_deploy" "--deployer-folder=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list (PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")



FetchContent_Declare(stc
    GIT_REPOSITORY https://github.com/LunarWatcher/stc
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(
    stc
)

find_package(OpenSSL REQUIRED)
find_package(Crow REQUIRED)
find_package(cpr REQUIRED)
find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)
find_package(libpqxx REQUIRED)
find_package(reflectcpp REQUIRED)

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
    COMMAND jade run
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
    COMMAND tests
    DEPENDS tests test-frontend
    COMMENT "Run backend and frontend tests")
# vim:ft=cmake
