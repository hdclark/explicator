
cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)
project (Explicator)
set (Explicator_VERSION_MAJOR 0)
set (Explicator_VERSION_MINOR 7)
set (Explicator_VERSION_PATCH 0)

# Set the release type. 
set(CMAKE_BUILD_TYPE Release) # Comment this line to use fallback default.
if(NOT CMAKE_BUILD_TYPE)
    # Default to debug builds.
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(BUILD_SHARED_LIBS TRUE)
set(POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy et al.


# Use the directory where CMakeLists.txt is for inclusions.
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(sample_lexicons)

include(CPack)

