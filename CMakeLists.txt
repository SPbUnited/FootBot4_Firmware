cmake_minimum_required(VERSION 3.16)

project(fb4_firmware LANGUAGES CXX)

# Set your C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D WEBOTS_INTEGRATION -D FIRMWARE_VERSION='\"webots\"'")


if(NOT DEFINED WEBOTS_HOME OR WEBOTS_HOME STREQUAL "")
  set(WEBOTS_HOME "$ENV{WEBOTS_HOME}")
endif()

# Let user/system override this:
# if(NOT DEFINED WEBOTS_HOME)
#   set(WEBOTS_HOME "" CACHE PATH "Path to Webots installation")
# endif()

# if(NOT WEBOTS_HOME OR NOT EXISTS "${WEBOTS_HOME}")
#   message(FATAL_ERROR "Set WEBOTS_HOME to your Webots install path, e.g. -DWEBOTS_HOME=/opt/webots")
# endif()

message(STATUS "Webots home: ${WEBOTS_HOME}")

# Where to find headers like #include "..."
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/lib/vt-linalg/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/half_float)
include_directories(${CMAKE_SOURCE_DIR}/lib/letter-shell/src)
include_directories(${CMAKE_SOURCE_DIR}/lib/letter-shell/src/extensions/cpp_support)
include_directories(${CMAKE_SOURCE_DIR}/lib/letter-shell/src/extensions/log)
include_directories(${WEBOTS_HOME}/include/controller/cpp)

# Source files
file(GLOB SOURCES
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/src/devices/*.cpp
  ${CMAKE_SOURCE_DIR}/lib/*.cpp
)

# Build a native executable (host compiler)
add_executable(${PROJECT_NAME} ${SOURCES})

# If you later add libraries, you’d link them here, e.g.:
# target_link_libraries(${PROJECT_NAME} PRIVATE someLib)


# --- Link Webots controller library ---
# Common library locations (platform-dependent):
#   Linux:   ${WEBOTS_HOME}/lib/controller/libcontroller.so (or similar)
#   macOS:   ${WEBOTS_HOME}/lib/controller/libcontroller.dylib
#   Windows: ${WEBOTS_HOME}/lib/controller/controller.lib / .dll import lib
#
# Update this line after checking your Webots install for the exact filename.
# target_link_libraries(MyController PRIVATE controller)

# If CMake cannot resolve "controller", explicitly use the full path:
# target_link_libraries(MyController PRIVATE
#   "${WEBOTS_HOME}/lib/controller/libcontroller.so"
# )
target_link_libraries(${PROJECT_NAME} PRIVATE
  "${WEBOTS_HOME}/lib/controller/libCppController.so"
)

# Optional: put the binary into build/release (nice for your structure)
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release"
)
