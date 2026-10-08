cmake_minimum_required(VERSION 3.27)

project(hyprtoggle
    DESCRIPTION "Hyprland plugin to make windows fullscreen or floating by dragging to screen edges"
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC "src/*.cpp")

add_library(hyprtoggle SHARED ${SRC})

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)

target_link_libraries(hyprtoggle PRIVATE rt PkgConfig::deps)

install(TARGETS hyprtoggle DESTINATION lib)
