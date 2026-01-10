cmake_minimum_required(VERSION 3.10)

project(open-AITD CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

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

#ZLIB
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(ZLIB_COMPAT ON)
#find_package(zlib-ng CONFIG REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libs/zlib-ng")
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/zlib-ng ${CMAKE_CURRENT_BINARY_DIR}/zlib-ng EXCLUDE_FROM_ALL)
set(ZLIB_FOUND TRUE)
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/zlib-ng)
set(ZLIB_LIBRARY zlibstatic)
#add_library(ZLIB::ZLIB ALIAS zlibstatic)
add_library(ZLIB::ZLIB INTERFACE IMPORTED)
target_link_libraries(ZLIB::ZLIB INTERFACE zlibstatic)

#PNG
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(PNG_TOOLS OFF)
set(PNG_BUILD_ZLIB OFF)
#find_package(PNG CONFIG REQUIRED PATHS "${CMAKE_SOURCE_DIR}/libs/png")
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/png ${CMAKE_CURRENT_BINARY_DIR}/png EXCLUDE_FROM_ALL)
add_library(PNG::PNG ALIAS png_static)

#LUA
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/lua2 ${CMAKE_CURRENT_BINARY_DIR}/lua EXCLUDE_FROM_ALL)
set(LUA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/lua2/src")

#LUACPP
#add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/luacpp ${CMAKE_CURRENT_BINARY_DIR}/luacpp )
set(LUACPP_INCLUDE_DIR "libs/luacpp/include")
file(GLOB_RECURSE LUACPP_SOURCES CONFIGURE_DEPENDS "libs/luacpp/src/*.cpp")

#RAYLIB
set(RAYLIB_INCLUDE_DIR "libs/raylib/src")
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/raylib ${CMAKE_CURRENT_BINARY_DIR}/raylib EXCLUDE_FROM_ALL)
#file(GLOB_RECURSE RAYLIB_SOURCES CONFIGURE_DEPENDS "libs/raylib/src/*.c")

#JSON
set(COMPILE_AS_CPP ON)
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/json ${CMAKE_CURRENT_BINARY_DIR}/json EXCLUDE_FROM_ALL)
set(JSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/json/include")

#TINYGLTF
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/tinygltf ${CMAKE_CURRENT_BINARY_DIR}/tinygltf EXCLUDE_FROM_ALL)
set(TINYGLTF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/tinygltf")

#OGG
set(OGG_INCLUDE_DIR "libs/ogg/include")
set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/ogg ${CMAKE_CURRENT_BINARY_DIR}/ogg EXCLUDE_FROM_ALL)

#VORBIS
set(VORBIS_INCLUDE_DIR "libs/vorbis/include")
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/vorbis ${CMAKE_CURRENT_BINARY_DIR}/vorbis EXCLUDE_FROM_ALL)


set(ALL_INCLUDES 
    ${ZLIB_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${GLFW_INCLUDE}
    ${LUA_INCLUDE_DIR}
    ${LUACPP_INCLUDE_DIR}
    ${RAYLIB_INCLUDE_DIR}
    ${JSON_INCLUDE_DIR}
    ${TINYGLTF_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${VORBIS_INCLUDE_DIR})

add_library(libextractor STATIC)
file(GLOB_RECURSE EXTRACTOR_SOURCES CONFIGURE_DEPENDS "src/extractor/*.cpp" "src/extractor/*.h" "src/extractor/*.hpp")
target_sources(libextractor PRIVATE ${EXTRACTOR_SOURCES}  ${RAYLIB_SOURCES})
target_include_directories(libextractor PRIVATE ${ALL_INCLUDES})
target_link_libraries(libextractor PRIVATE PNG::PNG ZLIB::ZLIB Ogg::ogg Vorbis::vorbis )

add_executable(extractor)
#target_compile_definitions(${PROJECT_NAME} PRIVATE XXX)
target_sources(extractor PRIVATE "src/extractor-cli/main.cpp" )
target_include_directories(extractor PRIVATE ${ALL_INCLUDES})
target_link_libraries(extractor PRIVATE PNG::PNG ZLIB::ZLIB Ogg::ogg Vorbis::vorbis libextractor )
set_property(TARGET extractor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")

add_executable(open-aitd)
file(GLOB_RECURSE AITD_SOURCES CONFIGURE_DEPENDS "src/engine/*.cpp" "src/engine/*.h" "src/engine/*.hpp")
target_sources(open-aitd PRIVATE ${AITD_SOURCES} ${LUACPP_SOURCES} ${RAYLIB_SOURCES})
target_include_directories(open-aitd PRIVATE ${ALL_INCLUDES})
#target_include_directories(open-aitd PRIVATE ${CMAKE_SOURCE_DIR}/libs/includes)
target_link_libraries(open-aitd PRIVATE PNG::PNG ZLIB::ZLIB lua::lua Ogg::ogg Vorbis::vorbis libextractor raylib)
set_property(TARGET open-aitd PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")

#INSTALL
install(TARGETS open-aitd RUNTIME DESTINATION .)
install(TARGETS extractor RUNTIME DESTINATION .)
install(DIRECTORY newdata/ DESTINATION newdata)

# get_target_property(ZLIB_DLL ZLIB::ZLIB IMPORTED_LOCATION_RELEASE)
# if(ZLIB_DLL AND EXISTS "${ZLIB_DLL}")
# 	install(FILES ${ZLIB_DLL} DESTINATION . )
# endif()
