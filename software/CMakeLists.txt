cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(PROJECT_NAME "digilivolo")
project(${PROJECT_NAME} C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu11 -flto -ffunction-sections -fdata-sections -ffat-lto-objects -Wall -Wl,--warn-common -Wl,--gc-sections")
set(HIDAPI_WITH_LIBUSB FALSE)
set(BUILD_SHARED_LIBS FALSE) # HIDAPI as static library on all platforms

if(POLICY CMP0074)
    # allow using hidapi_ROOT if CMake supports it
    cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0115)
    cmake_policy(SET CMP0115 NEW)
endif()

# Get version from git
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Project: ${PROJECT_NAME} ${GIT_VERSION}")

if(NOT USE_SYSTEM_HIDAPI)
    add_subdirectory(lib/hidapi)
    message(STATUS "hidapi will be built from sources")
else()
    message(STATUS "Finding library hidapi")
    find_package(hidapi 0.12 REQUIRED)
endif()

message(STATUS "Using HIDAPI: ${hidapi_VERSION}")

add_subdirectory(lib/argp-standalone)

configure_file(src/git_version.h.in src/git_version.h @ONLY)
add_executable(${PROJECT_NAME} src/digilivolo.c)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")
target_link_libraries(${PROJECT_NAME} hidapi::hidapi argp-standalone)

# Strip binary for release builds
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>
  VERBATIM
)
