cmake_minimum_required(VERSION 3.25)

project(CompositorWindows VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(compositor_core STATIC
  src/core/project_manifest.cpp
)

target_include_directories(compositor_core PUBLIC src)
target_link_libraries(compositor_core PUBLIC nlohmann_json::nlohmann_json)

if(MSVC)
  target_compile_options(compositor_core PRIVATE /W4 /permissive- /utf-8)
else()
  target_compile_options(compositor_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
  add_executable(Compositor WIN32
    src/app/main.cpp
  )
  target_compile_definitions(Compositor PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
  target_link_libraries(Compositor PRIVATE compositor_core user32 gdi32 comdlg32 shell32)
  if(MSVC)
    target_compile_options(Compositor PRIVATE /W4 /permissive- /utf-8)
  endif()
endif()

enable_testing()

add_executable(project_manifest_tests
  tests/project_manifest_tests.cpp
)
target_link_libraries(project_manifest_tests PRIVATE compositor_core)
add_test(NAME project_manifest_tests COMMAND project_manifest_tests)
