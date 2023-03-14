#
# Copyright 2021-2023 Software Radio Systems Limited
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the distribution.
#

# - Try to find sctp
#
# Once done this will define
#  SCTP_FOUND        - System has mbedtls
#  SCTP_INCLUDE_DIRS - The mbedtls include directories
#  SCTP_LIBRARIES    - The mbedtls library

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(PC_SCTP sctp)

FIND_PATH(
    SCTP_INCLUDE_DIRS
    NAMES netinet/sctp.h
    HINTS ${PC_SCTP_INCLUDEDIR}
          ${CMAKE_INSTALL_PREFIX}/include
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SCTP_LIBRARIES
    NAMES sctp
    HINTS ${PC_SCTP_LIBDIR}
          ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          /usr/lib/x86_64-linux-gnu/
)

message(STATUS "SCTP LIBRARIES: " ${SCTP_LIBRARIES})
message(STATUS "SCTP INCLUDE DIRS: " ${SCTP_INCLUDE_DIRS})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SCTP DEFAULT_MSG SCTP_LIBRARIES SCTP_INCLUDE_DIRS)
MARK_AS_ADVANCED(SCTP_LIBRARIES SCTP_INCLUDE_DIRS)
