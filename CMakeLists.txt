cmake_minimum_required (VERSION 2.6)

project(FLOBBY)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(WITH_PRD "Build pr-downloader" ON)

# Set the default build type to 'Release' if none was specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")

# fltk static library
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fltk/CMakeLists.txt")
    MESSAGE(FATAL_ERROR "fltk/CMakeLists.txt doesn't exist, run \ngit submodule update --init")
else()
    # add include path for FL/abi-version.h
    include_directories (${CMAKE_BINARY_DIR}/fltk)
    add_subdirectory (fltk EXCLUDE_FROM_ALL)
endif()

add_subdirectory (src)

# TODO enable below if/when linking to pr-d static
#include_directories (
#    ${PROJECT_SOURCE_DIR}/pr-downloader/src
#)

if (WITH_PRD)

# build pr-downloader executable, link statically
set(PRD_CONSOLETOOL ON CACHE BOOL "build console tool")
set(PRD_SHAREDLIB OFF CACHE BOOL "build shared lib")
set(PRD_STATICLIB ON CACHE BOOL "build static lib")
set(PRD_LINK_STATIC ON CACHE BOOL "link to static lib")
set(PRD_DEVELOP_FILES OFF CACHE BOOL "install develop files")
set(PRD_DO_INSTALL ON CACHE BOOL "install any pr-downloader files")
set(PRD_ARCHIVE_SUPPORT ON CACHE BOOL "enable archive support")
set(PRD_TESTS OFF CACHE BOOL "Enable prd unit tests")
set(PRD_JSONCPP_INTERNAL OFF CACHE BOOL "use bundled JsonCpp")



mark_as_advanced(FORCE
    PRD_CLEAR_COMPILER_FLAGS
    PRD_CONSOLETOOL
    PRD_SHAREDLIB
    PRD_STATICLIB
    PRD_DEVELOP_FILES
    PRD_DO_INSTALL
    PRD_ARCHIVE_SUPPORT
    PRD_LINK_STATIC
    PRD_LIBDIR
    PRD_BINDIR
    PRD_INCLUDE_DIR
    PRD_ENABLE_LSL
    PRD_JSONCPP_INTERNAL
    PRD_TESTS
)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pr-downloader/CMakeLists.txt")
    MESSAGE(FATAL_ERROR "pr-downloader/CMakeLists.txt doesn't exist, run \ngit submodule update --init\nif you want to build pr-downloader or turn off WITH_PRD")
else()
    add_subdirectory(pr-downloader)
endif()

endif(WITH_PRD)
