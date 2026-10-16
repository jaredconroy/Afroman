cmake_minimum_required(VERSION 3.20)
project(AfromanWeb LANGUAGES C CXX)
if(NOT EMSCRIPTEN)
 message(FATAL_ERROR "Configure using Emscripten emcmake")
endif()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(ENGINE "${ROOT}/vendor/cocos2d/cocos")
set(THIRD "${CMAKE_CURRENT_LIST_DIR}/third_party")
add_compile_options(-UEMSCRIPTEN -DGL_GLEXT_PROTOTYPES -sFULL_ES2=1 -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_FREETYPE=1)
add_compile_definitions(CC_USE_PHYSICS=0 CC_ENABLE_SCRIPT_BINDING=0 CC_TEXTURE_ATLAS_USE_VAO=0 COCOS2D_DEBUG=1)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>)
# These optional engine subsystems are not used by Afroman. Keep the 2D renderer,
# image codecs, text, TMX, actions, scheduler, and event system intact.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(tiff-tools OFF CACHE BOOL "" FORCE)
set(tiff-tests OFF CACHE BOOL "" FORCE)
set(tiff-contrib OFF CACHE BOOL "" FORCE)
set(tiff-docs OFF CACHE BOOL "" FORCE)
set(zlib OFF CACHE BOOL "" FORCE)
set(jpeg OFF CACHE BOOL "" FORCE)
set(webp OFF CACHE BOOL "" FORCE)
set(lzma OFF CACHE BOOL "" FORCE)
set(zstd OFF CACHE BOOL "" FORCE)
set(jbig OFF CACHE BOOL "" FORCE)
set(lerc OFF CACHE BOOL "" FORCE)
add_subdirectory("${ROOT}/vendor/libtiff" tiff EXCLUDE_FROM_ALL)
set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_CWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_DWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_VWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
add_subdirectory("${ROOT}/vendor/libwebp" webp EXCLUDE_FROM_ALL)
set(SOURCES "${ENGINE}/cocos2d.cpp")
foreach(component base 2d renderer math deprecated)
 include("${ENGINE}/${component}/CMakeLists.txt")
 string(TOUPPER "${component}" upper)
 foreach(source IN LISTS COCOS_${upper}_SRC)
  list(APPEND SOURCES "${ENGINE}/${source}")
 endforeach()
endforeach()
list(APPEND SOURCES
 "${ENGINE}/platform/CCFileUtils.cpp" "${ENGINE}/platform/CCImage.cpp"
 "${ENGINE}/platform/CCSAXParser.cpp" "${ENGINE}/platform/CCGLViewProtocol.cpp"
 "${THIRD}/tinyxml2/tinyxml2.cpp" "${THIRD}/unzip/unzip.cpp" "${THIRD}/unzip/ioapi.cpp"
 "${THIRD}/xxhash/xxhash.c" "${THIRD}/edtaa3func/edtaa3func.cpp"
 "${THIRD}/ConvertUTF/ConvertUTF.c" "${THIRD}/ConvertUTF/ConvertUTFWrapper.cpp")
file(GLOB PLATFORM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/platform/*.cpp")
add_library(cocos_web STATIC ${SOURCES} ${PLATFORM_SOURCES})
target_include_directories(cocos_web PUBLIC
 "${CMAKE_CURRENT_LIST_DIR}/platform" "${ENGINE}" "${ENGINE}/2d" "${ENGINE}/platform"
 "${THIRD}/tinyxml2" "${THIRD}/unzip" "${THIRD}/xxhash" "${THIRD}/ConvertUTF"
 "${THIRD}/edtaa3func" "${ROOT}/vendor/libwebp/src/webp"
 "${ROOT}/vendor/libtiff/libtiff" "${CMAKE_CURRENT_BINARY_DIR}/tiff/libtiff")
target_link_libraries(cocos_web PUBLIC tiff webp)
add_executable(afroman main.cpp)
target_link_libraries(afroman PRIVATE cocos_web)
set_target_properties(afroman PROPERTIES SUFFIX ".html")
target_link_options(afroman PRIVATE -sFULL_ES2=1 -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_FREETYPE=1
 -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=2 -sGL_ASSERTIONS=1 -sSTACK_SIZE=1048576
 "--shell-file=${CMAKE_CURRENT_LIST_DIR}/shell.html"
 "--preload-file=${ROOT}/Resources@/Resources")

set_property(TARGET afroman APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/shell.html")
