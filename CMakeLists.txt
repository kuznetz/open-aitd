cmake_minimum_required(VERSION 3.5)
project(open-AITD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#ZLib-NG
find_package(ZLIB REQUIRED PATHS libs/zlib-ng-2.2.4)
set( ZLIB_LIBRARY ZLIB::ZLIB )
message ( STATUS "DIR ${ZLIB_INCLUDE_DIRS}" )

# Adding libpng
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(PNG_TOOLS OFF)
set(PNG_BUILD_ZLIB OFF)
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/libpng ${CMAKE_CURRENT_BINARY_DIR}/libs/libpng )
#set(PNG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/libpng )
#message ( STATUS "DIR ${PNG_INCLUDE_DIR}" )

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/raylib ${CMAKE_CURRENT_BINARY_DIR}/libs/raylib )
set(GLFW_INCLUDE "${CMAKE_SOURCE_DIR}/libs/raylib/external/glfw/include/GLFW")

# Adding our source files
# set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

add_library(libextractor STATIC)
file(GLOB_RECURSE EXTRACTOR_SOURCES CONFIGURE_DEPENDS "src/extractor/*.cpp" "src/extractor/*.h" "src/extractor/*.hpp")
target_sources(libextractor PRIVATE ${EXTRACTOR_SOURCES})
target_include_directories(libextractor PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_include_directories(libextractor PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(libextractor PRIVATE png_static raylib ZLIB::ZLIB )

add_executable(extractor_cli)
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
target_sources(extractor_cli PRIVATE "src/extractor-cli/main.cpp" )
target_include_directories(extractor_cli PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_include_directories(extractor_cli PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(extractor_cli PRIVATE png_static raylib ZLIB::ZLIB libextractor )
set_property(TARGET extractor_cli PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")

#add_executable(open-aitd) WIN32
add_executable(open-aitd)
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
file(GLOB_RECURSE AITD_SOURCES CONFIGURE_DEPENDS "src/engine/*.cpp" "src/engine/*.h" "src/engine/*.hpp")
target_sources(open-aitd PRIVATE ${AITD_SOURCES} )
target_include_directories(open-aitd PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_include_directories(open-aitd PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(open-aitd PRIVATE png_static raylib ZLIB::ZLIB libextractor )
set_property(TARGET open-aitd PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")