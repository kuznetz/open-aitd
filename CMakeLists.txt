cmake_minimum_required(VERSION 3.5)
message(STATUS "VCPKG_ROOT = $ENV{VCPKG_ROOT}")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(open-AITD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Disable asserts
    add_compile_definitions(NDEBUG)
	# For now - disable warns
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)	
    if(MSVC)
        add_compile_options(/O2 /Ob2 /Oi /Ot /Oy /GL)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
        set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
        add_compile_options(/sdl /guard:cf)
    else()
        add_compile_options(-O3 -march=native -flto=auto -fomit-frame-pointer)
        add_compile_options(-D_FORTIFY_SOURCE=2 -fstack-protector-strong)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-z,relro,-z,now")
        if(ENABLE_LTO)
            add_compile_options(-flto)
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
        endif()
    endif()
#Debug
else()
    add_compile_definitions(_DEBUG)
    if(MSVC)
        add_compile_options(/Zi /Od)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

# vcpkg path
set(VCPKG_INSTALLED_DIR "vcpkg_installed")
set(VCPKG_TARGET_TRIPLET "x64-windows") #x64-linux, x64-osx
list(APPEND CMAKE_PREFIX_PATH 
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share"
)

#VCPKG
find_package(PNG CONFIG REQUIRED)
find_package(zlib-ng CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
#find_package(raylib CONFIG REQUIRED)

#LUA
set(LUA_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
set(LUA_LIBRARIES "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
#set(COMPILE_AS_CPP ON)
find_package(Lua REQUIRED)
message(STATUS "Lua version: ${LUA_VERSION_STRING}")
message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libraries: ${LUA_LIBRARIES}")

#LUACPP
#add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/luacpp ${CMAKE_CURRENT_BINARY_DIR}/luacpp )
set(LUACPP_INCLUDE_DIR "libs/luacpp/include")
file(GLOB_RECURSE LUACPP_SOURCES CONFIGURE_DEPENDS "libs/luacpp/src/*.cpp")

#RAYLIB
set(RAYLIB_INCLUDE_DIR "libs/raylib/src")
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/raylib ${CMAKE_CURRENT_BINARY_DIR}/raylib )
#file(GLOB_RECURSE RAYLIB_SOURCES CONFIGURE_DEPENDS "libs/raylib/src/*.c")


add_library(libextractor STATIC)
file(GLOB_RECURSE EXTRACTOR_SOURCES CONFIGURE_DEPENDS "src/extractor/*.cpp" "src/extractor/*.h" "src/extractor/*.hpp")
target_sources(libextractor PRIVATE ${EXTRACTOR_SOURCES}  ${RAYLIB_SOURCES})
target_include_directories(libextractor PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE} ${RAYLIB_INCLUDE_DIR})
target_link_libraries(libextractor PRIVATE PNG::PNG ZLIB::ZLIB )

add_executable(extractor)
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
target_sources(extractor PRIVATE "src/extractor-cli/main.cpp" )
target_include_directories(extractor PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE})
target_include_directories(extractor PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(extractor PRIVATE PNG::PNG ZLIB::ZLIB libextractor )
set_property(TARGET extractor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")

add_executable(open-aitd)
file(GLOB_RECURSE AITD_SOURCES CONFIGURE_DEPENDS "src/engine/*.cpp" "src/engine/*.h" "src/engine/*.hpp")
target_sources(open-aitd PRIVATE ${AITD_SOURCES} ${LUACPP_SOURCES} ${RAYLIB_SOURCES})
target_include_directories(open-aitd PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${GLFW_INCLUDE} ${LUA_INCLUDE_DIR} ${LUACPP_INCLUDE_DIR} ${RAYLIB_INCLUDE_DIR})
#target_include_directories(open-aitd PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(open-aitd PRIVATE PNG::PNG ZLIB::ZLIB libextractor lua raylib)
set_property(TARGET open-aitd PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")