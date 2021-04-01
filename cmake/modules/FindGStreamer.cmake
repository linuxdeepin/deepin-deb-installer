# - Try to find GStreamer
# Once done this will define
#
#  GSTREAMER_FOUND - system has GStreamer
#  GSTREAMER_INCLUDE_DIR - the GStreamer include directory
#  GSTREAMER_LIBRARIES - the libraries needed to use GStreamer
#  GSTREAMER_DEFINITIONS - Compiler switches required for using GStreamer
#  GSTREAMER_VERSION - the version of GStreamer

# Copyright (c) 2008 Helio Chissini de Castro, <helio@kde.org>
#  (c)2006, Tim Beaulen <tbscope@gmail.com>

# TODO: Other versions --> GSTREAMER_X_Y_FOUND (Example: GSTREAMER_0_8_FOUND and GSTREAMER_1.0_FOUND etc)


IF (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)
   # in cache already
   SET(GStreamer_FIND_QUIETLY TRUE)
ELSE (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)
   SET(GStreamer_FIND_QUIETLY FALSE)
ENDIF (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)

IF (NOT WIN32)
   FIND_PACKAGE(PkgConfig REQUIRED)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   # don't make this check required - otherwise you can't use macro_optional_find_package on this one
   PKG_CHECK_MODULES(PKG_GSTREAMER gstreamer-1.0)
   SET(GSTREAMER_VERSION ${PKG_GSTREAMER_VERSION})
   SET(GSTREAMER_DEFINITIONS ${PKG_GSTREAMER_CFLAGS})
ENDIF (NOT WIN32)

FIND_PATH(GSTREAMER_INCLUDE_DIR gst/gst.h
   PATHS
   ${PKG_GSTREAMER_INCLUDE_DIRS}
   PATH_SUFFIXES gstreamer-1.0
   )

FIND_LIBRARY(GSTREAMER_LIBRARIES NAMES gstreamer-1.0
   PATHS
   ${PKG_GSTREAMER_LIBRARY_DIRS}
   )

FIND_LIBRARY(GSTREAMER_BASE_LIBRARY NAMES gstbase-1.0
   PATHS
   ${PKG_GSTREAMER_LIBRARY_DIRS}
   )

FIND_LIBRARY(GSTREAMER_APP_LIBRARY NAMES gstapp-1.0
   PATHS
   ${PKG_GSTREAMER_LIBRARY_DIRS}
   )

IF (GSTREAMER_INCLUDE_DIR)
ELSE (GSTREAMER_INCLUDE_DIR)
   MESSAGE(STATUS "GStreamer: WARNING: include dir not found")
ENDIF (GSTREAMER_INCLUDE_DIR)

IF (GSTREAMER_LIBRARIES)
ELSE (GSTREAMER_LIBRARIES)
   MESSAGE(STATUS "GStreamer: WARNING: library not found")
ENDIF (GSTREAMER_LIBRARIES)

if (GSTREAMER_APP_LIBRARY)
ELSE (GSTREAMER_APP_LIBRARY)
   MESSAGE(STATUS "GStreamer: WARNING: app library not found")
ENDIF (GSTREAMER_APP_LIBRARY)

IF (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)
   SET(GSTREAMER_FOUND TRUE)
ELSE (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)
   SET(GSTREAMER_FOUND FALSE)
ENDIF (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES AND GSTREAMER_BASE_LIBRARY AND GSTREAMER_APP_LIBRARY)

IF (GSTREAMER_FOUND)
   IF (NOT GStreamer_FIND_QUIETLY)
      MESSAGE(STATUS "Found GStreamer: ${GSTREAMER_LIBRARIES}")
   ENDIF (NOT GStreamer_FIND_QUIETLY)
ELSE (GSTREAMER_FOUND)
   IF (GStreamer_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find GStreamer")
   ENDIF (GStreamer_FIND_REQUIRED)
ENDIF (GSTREAMER_FOUND)

MARK_AS_ADVANCED(GSTREAMER_INCLUDE_DIR GSTREAMER_LIBRARIES GSTREAMER_BASE_LIBRARY GSTREAMER_INTERFACE_LIBRARY GSTREAMER_APP_LIBRARY)
