cmake_minimum_required(VERSION 3.10.0)
project(sdl-c8 VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS})

add_executable(sdl-c8 Chip8.cpp SDL_MainComponents.cpp main.cpp SDLBeep.cpp Configuration.cpp)
target_link_libraries(sdl-c8 ${SDL3_LIBRARIES})
set_target_properties(sdl-c8 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")


include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
