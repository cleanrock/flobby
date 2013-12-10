cmake_minimum_required (VERSION 2.6)

project(FLOBBY)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set the default build type to 'Release' if none was specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)


include_directories (
    ${PROJECT_SOURCE_DIR}/pr-downloader/src
)

add_subdirectory (src)

# pr-downloader static lib
set(PRD_CONSOLETOOL OFF CACHE BOOL "disable console tool compile" FORCE)
set(PRD_SHAREDLIB OFF CACHE BOOL "disable shared lib compile" FORCE)
set(PRD_DEVELOP_FILES OFF CACHE BOOL "disable develop files" FORCE)
set(PRD_DO_INSTALL OFF CACHE BOOL "don't install any pr-downloader file" FORCE)
set(PRD_ARCHIVE_SUPPORT OFF CACHE BOOL "disable archive support as it interferes with unitsync!" FORCE)
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
    MESSAGE(FATAL_ERROR "pr-downloader/CMakeLists.txt doesn't exist, please run \ngit submodule update --init")
else()
    add_subdirectory(pr-downloader)
endif()
