cmake_minimum_required(VERSION 3.5)
message(STATUS "VCPKG_ROOT = $ENV{VCPKG_ROOT}")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(open-AITD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установите путь к vcpkg установленным пакетам
set(VCPKG_INSTALLED_DIR "vcpkg_installed")
set(VCPKG_TARGET_TRIPLET "x64-windows") #x64-linux, x64-osx
list(APPEND CMAKE_PREFIX_PATH 
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share"
)

#
find_package(PNG CONFIG REQUIRED)
find_package(zlib-ng CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)

#LUA
set(LUA_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
set(LUA_LIBRARIES "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
#set(COMPILE_AS_CPP ON)
#find_package(Lua REQUIRED)
message(STATUS "Lua version: ${LUA_VERSION_STRING}")
message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libraries: ${LUA_LIBRARIES}")

#LUACPP
#add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/luacpp ${CMAKE_CURRENT_BINARY_DIR}/luacpp )

add_library(libextractor STATIC)
file(GLOB_RECURSE EXTRACTOR_SOURCES CONFIGURE_DEPENDS "src/extractor/*.cpp" "src/extractor/*.h" "src/extractor/*.hpp")
target_sources(libextractor PRIVATE ${EXTRACTOR_SOURCES})
target_include_directories(libextractor PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_link_libraries(libextractor PRIVATE PNG::PNG raylib ZLIB::ZLIB )

add_executable(extractor_cli)
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
target_sources(extractor_cli PRIVATE "src/extractor-cli/main.cpp" )
target_include_directories(extractor_cli PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_include_directories(extractor_cli PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(extractor_cli PRIVATE PNG::PNG raylib ZLIB::ZLIB libextractor )
set_property(TARGET extractor_cli PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")
