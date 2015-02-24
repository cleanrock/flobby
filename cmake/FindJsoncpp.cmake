# -*- cmake -*-

# - Find JSONCpp
# Find the JSONCpp includes and library
# This module defines
#  JSONCPP_FOUND, System has libjsoncpp.
#  JSONCPP_INCLUDE_DIRS - The libjsoncpp include directories.
#  JSONCPP_LIBRARIES - The libraries needed to use libjsoncpp.
#  JSONCPP_DEFINITIONS - Compiler switches required for using libjsoncpp.

find_package(PkgConfig)

pkg_check_modules(PC_JSONCPP jsoncpp)

SET(JSONCPP_DEFINITIONS ${PC_JSONCPP_CFLAGS_OTHER})

# include dir
FIND_PATH(JSONCPP_INCLUDE_DIR json/reader.h
  HINTS ${PC_JSONCPP_INCLUDE_DIR} ${PC_JSONCPP_INCLUDE_DIRS}
  PATH_SUFFIXES jsoncpp
)
  
# library
FIND_LIBRARY(JSONCPP_LIBRARY
  NAMES jsoncpp
  HINTS ${PC_JSONCPP_LIBDIR} ${PC_JSONCPP_LIBRARY_DIRS}
  PATHS /usr/lib /usr/local/lib
)

SET(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
SET(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(JSONCPP DEFAULT_MSG JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR)

MARK_AS_ADVANCED(JSONCPP_LIBRARY JSONCPP_INCLUDE_DIR)
