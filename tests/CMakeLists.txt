find_package(Catch2 3 REQUIRED)

include_directories("${Edit_SOURCE_DIR}/src")

add_executable(testlib testlib.cpp)

target_link_libraries(testlib PRIVATE Catch2::Catch2WithMain SDL3_ttf::SDL3_ttf SDL3::SDL3)
