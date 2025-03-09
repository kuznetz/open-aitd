cmake_minimum_required(VERSION 3.5)
project(open-AITD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#message ( STATUS "DIR ${CMAKE_SOURCE_DIR}/libs/zlib" )

#set(ZLIB_BUILD_EXAMPLES OFF)
#set(ZLIB_BUILD_TESTING OFF)
#set(ZLIB_BUILD_SHARED ON)
#set(ZLIB_BUILD_STATIC ON)
#set(ZLIB_BUILD_MINIZIP OFF)
#add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/zlib ${CMAKE_CURRENT_BINARY_DIR}/libs/zlib )

#set(ZLIB_ROOT ${CMAKE_SOURCE_DIR}/libs/zlib)
#set(ZLIB_USE_STATIC_LIBS ON)
#find_package(ZLIB REQUIRED)
#add_library(ZLIB::ZLIB ALIAS zlibstatic)
#set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libs/zlib)
#set(ZLIB_LIBRARY_2 ${CMAKE_CURRENT_BINARY_DIR}/libs/zlib/Debug/zsd.lib )
#get_property(ZLIB_LIBRARY TARGET zlibstatic PROPERTY OUTPUT_NAME)
#message ( STATUS "ZLIB_LIBRARY ${ZLIB_LIBRARY}" )
#add_library(ZLIB::ZLIB ALIAS zlibstatic)
#get_property(ZLIB_LIBRARY TARGET zlibstatic PROPERTY OUTPUT_NAME)

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

# Adding Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/raylib ${CMAKE_CURRENT_BINARY_DIR}/libs/raylib )
set(GLFW_INCLUDE "${CMAKE_SOURCE_DIR}/libs/raylib/external/glfw/include/GLFW")

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.hpp")
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

add_executable(${PROJECT_NAME})
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES} )
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE} ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE png_static raylib ZLIB::ZLIB )

target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") 