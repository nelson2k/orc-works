cmake_minimum_required(VERSION 3.16)
project(orcgui-wx CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

set(wxWidgets_DIR
    "${CMAKE_SOURCE_DIR}/../repos-folder/wxWidgets-install/lib/cmake/wxWidgets-3.3"
    CACHE PATH "Path to wxWidgetsConfig.cmake (bundled install)")
find_package(wxWidgets CONFIG REQUIRED COMPONENTS mono)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(SOURCES
    src/main.cpp
    src/Worker.cpp
    src/Metrics.cpp
    src/VBar.cpp
    src/ZoomPanel.cpp
    src/FlatButton.cpp
)

if (WIN32)
    enable_language(RC)
    list(APPEND SOURCES src/orcgui.rc)
    set_source_files_properties(src/orcgui.rc PROPERTIES
        COMPILE_FLAGS
        "-I\"${CMAKE_SOURCE_DIR}/../repos-folder/wxWidgets-install/include\" -I\"${CMAKE_SOURCE_DIR}/../repos-folder/wxWidgets-install/lib/gcc_x64_lib/mswu\"")
endif()

add_executable(orcgui WIN32 ${SOURCES})

target_link_libraries(orcgui PRIVATE
    wx::mono
    nlohmann_json::nlohmann_json
    $<$<BOOL:${WIN32}>:dwmapi>
)

if (WIN32)
    target_compile_definitions(orcgui PRIVATE
        _UNICODE UNICODE
        NOMINMAX WIN32_LEAN_AND_MEAN
    )
endif()

add_custom_command(TARGET orcgui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/icons"
        "$<TARGET_FILE_DIR:orcgui>/icons"
    VERBATIM)
