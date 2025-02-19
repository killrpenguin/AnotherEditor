cmake_minimum_required(VERSION 3.16...3.31)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Declare the project
project(
  Edit
  VERSION 0.15
  DESCRIPTION "Another attempt at a GUI editor."
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_COMPILER "/usr/lib/llvm/19/bin/clang++")
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
  set(CMAKE_CXX_EXTENSIONS OFF)
  include(CTest)
endif()

set(SDL_SHARED ON)

set(EXECUTABLE_NAME ${PROJECT_NAME})

set(SRC_LIST 
    "${Edit_SOURCE_DIR}/src/main.cpp"
    "${Edit_SOURCE_DIR}/src/App.cpp")

add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${SRC_LIST})

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

set(SDLTTF_VENDORED ON)

add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3_ttf::SDL3_ttf SDL3::SDL3)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
  AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
