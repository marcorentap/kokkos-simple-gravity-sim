cmake_minimum_required(VERSION 3.16)
project(simplesim LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
message(STATUS "Fetching SFML")
FetchContent_MakeAvailable(SFML)
message(STATUS "Done fetching SFML")

find_package(Kokkos REQUIRED)
include_directories(include)
add_executable(simplesim src/grid.cpp src/main.cpp)
target_link_libraries(simplesim PRIVATE Kokkos::kokkos)
target_link_libraries(simplesim PRIVATE sfml-graphics)
target_compile_features(simplesim PRIVATE cxx_std_17)


if(WIN32)
    add_custom_command(
        TARGET simplesim
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:simplesim>
        VERBATIM)
endif()

install(TARGETS simplesim)
