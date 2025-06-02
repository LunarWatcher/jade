add_executable(tests 
    src/Main.cpp

    src/unit/InterruptableThread.cpp
    src/unit/SearchParsing.cpp
)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests Catch2::Catch2)

target_link_libraries(tests jadesrc)
target_include_directories(tests PUBLIC ./src/)


# vim:ft=cmake
