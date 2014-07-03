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


# TODO enable below if/when linking to pr-d static
#include_directories (
#    ${PROJECT_SOURCE_DIR}/pr-downloader/src
#)

find_package(Git REQUIRED)
EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} describe --dirty
	OUTPUT_VARIABLE FLOBBY_REV
	ERROR_VARIABLE GIT_ERROR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
SET(FLOBBY_REV "${FLOBBY_REV}")
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/cmake/config.h ${CMAKE_SOURCE_DIR}/src/FlobbyConfig.h )

add_subdirectory (src)

if (WITH_PRD)

# build pr-downloader executable, link statically
set(PRD_CONSOLETOOL ON CACHE BOOL "build console tool")
set(PRD_SHAREDLIB OFF CACHE BOOL "build shared lib")
set(PRD_STATICLIB ON CACHE BOOL "build static lib")
set(PRD_LINK_STATIC ON CACHE BOOL "link to static lib")
set(PRD_DEVELOP_FILES OFF CACHE BOOL "install develop files")
set(PRD_DO_INSTALL ON CACHE BOOL "install any pr-downloader files")
set(PRD_ARCHIVE_SUPPORT ON CACHE BOOL "archive support interferes with 91.0 unitsync when linking static")
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
)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pr-downloader/CMakeLists.txt")
    MESSAGE(FATAL_ERROR "pr-downloader/CMakeLists.txt doesn't exist, run \ngit submodule update --init\nif you want to build pr-downloader or turn off WITH_PRD")
else()
    add_subdirectory(pr-downloader)
endif()

endif(WITH_PRD)