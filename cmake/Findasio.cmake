message(STATUS "Sourcing Asio from GitHub...")

include(FetchContent)
# Note: This cannot always be latest asio. Crow seems to restrict supported versions quite a bit
set (ASIO_VERSION "asio-1-30-0")

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG ${ASIO_VERSION}
)

FetchContent_MakeAvailable(asio)

add_library(
    asio INTERFACE
)
add_library(asio::asio ALIAS asio)

target_include_directories(
    asio INTERFACE
    ${asio_SOURCE_DIR}/asio/include
)

set (ASIO_FOUND YES)
set (ASIO_LIBRARIES asio::asio)

# Hard disable boost
target_compile_definitions(asio INTERFACE
    -DASIO_DISABLE_BOOST_ALIGN
    -DASIO_DISABLE_BOOST_ARRAY
    -DASIO_DISABLE_BOOST_ASSERT
    -DASIO_DISABLE_BOOST_BIND
    -DASIO_DISABLE_BOOST_CHRONO
    -DASIO_DISABLE_BOOST_CONTEXT_FIBER
    -DASIO_DISABLE_BOOST_COROUTINE
    -DASIO_DISABLE_BOOST_DATE_TIME
    -DASIO_DISABLE_BOOST_LIMITS
    -DASIO_DISABLE_BOOST_REGEX
    -DASIO_DISABLE_BOOST_SOURCE_LOCATION
    -DASIO_DISABLE_BOOST_THROW_EXCEPTION
    -DASIO_DISABLE_BOOST_WORKAROUND
)
