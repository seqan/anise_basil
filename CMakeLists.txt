cmake_minimum_required (VERSION 2.8.2)
project (anise_basil)

# ----------------------------------------------------------------------------
# Project Configuration
# ----------------------------------------------------------------------------

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "CMake release mode." FORCE)
endif ()

# Set output binary directory.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# ----------------------------------------------------------------------------
# Prepare Dependency Search
# ----------------------------------------------------------------------------

# Prepare LEMON library.
# - make FindLEMON work
set (LEMON_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/ext/lemon)
# - create config.h file
configure_file (${CMAKE_SOURCE_DIR}/ext/lemon/lemon/config.h.in
                ${CMAKE_SOURCE_DIR}/ext/lemon/lemon/config.h)

# Prepare SEQAN library.
set (SEQAN_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/ext/seqan/include)
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/util/cmake
                       ${CMAKE_SOURCE_DIR}/ext/seqan/util/cmake)

# ----------------------------------------------------------------------------
# Include Subdirectory "src"
# ----------------------------------------------------------------------------

# Add subdirectory with the actual ANISE/BASIL programs.
add_subdirectory (src)

# ----------------------------------------------------------------------------
# Installation
# ----------------------------------------------------------------------------

# Install scripts.
install (PROGRAMS scripts/filter_basil.py
                  scripts/extract_spanning.awk
                  scripts/best_blat.py
         DESTINATION bin)

# Install example files.
install (FILES example/left.fq.gz
               example/right.fq.gz
               example/ref.fa
               example/ins.fa
         DESTINATION example)

# Install text files.
install (FILES LICENSE
               README.md
         DESTINATION .)

# ----------------------------------------------------------------------------
# CPack Configuration
# ----------------------------------------------------------------------------

set (CPACK_PACKAGE_NAME "anise_basil")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Methods for the prediction and assembly of large instartions.")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Manuel Holtgrewe <manuel.holtgrewe@fu-berlin.de>")
set (CPACK_PACKAGE_VENDOR "Manuel Holtgrewe <manuel.holtgrewe@fu-berlin.de>")

set (CPACK_PACKAGE_VERSION "1.1.0")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "1")
set (CPACK_PACKAGE_VERSION_PATCH "0")

set (CPACK_GENERATOR "TGZ")

include (CPack)
