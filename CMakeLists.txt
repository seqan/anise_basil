cmake_minimum_required (VERSION 2.8.2)
project (anise_basil)
message (STATUS "Configuring extras/apps/anise")

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "CMake release mode." FORCE)
endif ()

# Prepare LEMON library.
# (make FindLEMON work)
set (LEMON_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/ext/lemon")
# (create config.h file)
configure_file ("${CMAKE_SOURCE_DIR}/ext/lemon/lemon/config.h.in"
                "${CMAKE_SOURCE_DIR}/ext/lemon/lemon/config.h")

# Prepare SEQAN library.
set (SEQAN_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/ext/seqan/include")
#set (SEQAN_FIND_DEBUG TRUE)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/util/cmake" "${CMAKE_SOURCE_DIR}/ext/seqan/util/cmake")

# Set output binary directory.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Add subdirectory with the actual ANISE/BASIL programs.
add_subdirectory (src)
