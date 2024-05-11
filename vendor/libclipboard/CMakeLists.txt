# libclipboard cmake configuration file

cmake_minimum_required(VERSION 2.8)
project(libclipboard)

# Defines and options
set(LIBCLIPBOARD_VERSION_MAJOR 1 CACHE STRING "libclipboard major version number")
set(LIBCLIPBOARD_VERSION_MINOR 0 CACHE STRING "libclipboard minor version number")
set(LIBCLIPBOARD_VERSION "${LIBCLIPBOARD_VERSION_MAJOR}.${LIBCLIPBOARD_VERSION_MINOR}" CACHE STRING "libclipboard version number")

option(LIBCLIPBOARD_FORCE_WIN32 "Force building the Win32 backend (default:off)" OFF)
option(LIBCLIPBOARD_FORCE_X11 "Force building the X11 backend (default:off)" OFF)
option(LIBCLIPBOARD_FORCE_COCOA "Force building the Cocoa backend(default:off)" OFF)

option(LIBCLIPBOARD_ADD_SOVERSION "Add soname versions to the built library (default:off)" OFF)
option(LIBCLIPBOARD_USE_STDCALL "Use the stdcall calling convention (default:off)" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static libraries" OFF)
set(LIBCLIPBOARD_BUILD_SHARED ${BUILD_SHARED_LIBS})

# Sigh... gtest quirks workarounds
if (MINGW)
    option(gtest_disable_pthreads "Disable gtest pthreads (default:on)" ON)
elseif(MSVC)
    option(gtest_force_shared_crt "Always use dynamic runtime library (default:on)" ON)
endif()

# Check supplied options make sense
if ((WIN32 OR CYGWIN OR LIBCLIPBOARD_FORCE_WIN32) AND NOT (LIBCLIPBOARD_FORCE_X11 OR LIBCLIPBOARD_FORCE_COCOA))
    set(LIBCLIPBOARD_BUILD_WIN32 TRUE)
endif()

if (((UNIX AND NOT APPLE) OR LIBCLIPBOARD_FORCE_X11) AND NOT (LIBCLIPBOARD_FORCE_WIN32 OR LIBCLIPBOARD_FORCE_COCOA))
    set(LIBCLIPBOARD_BUILD_X11 TRUE)
endif()

if ((APPLE OR LIBCLIPBOARD_FORCE_COCOA) AND NOT (LIBCLIPBOARD_FORCE_WIN32 OR LIBCLIPBOARD_FORCE_X11))
    set(LIBCLIPBOARD_BUILD_COCOA TRUE)
endif()

if (NOT (LIBCLIPBOARD_BUILD_WIN32 OR LIBCLIPBOARD_BUILD_X11 OR LIBCLIPBOARD_BUILD_COCOA))
    message(FATAL_ERROR "Invalid build options. Can only specify one backend to be built.")
endif()

# Set compiler flags
if (CMAKE_COMPILER_IS_GNUCC OR LIBCLIPBOARD_BUILD_COCOA)
    set(GCC_COMPILE_FLAGS "-std=c99 -Wall -pedantic -g")

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -fdiagnostics-color=auto")
    endif()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR LIBCLIPBOARD_BUILD_COCOA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -g")
endif()

# Dependencies
if (LIBCLIPBOARD_BUILD_COCOA)
    set_source_files_properties(src/clipboard_cocoa.c PROPERTIES COMPILE_FLAGS "-x objective-c")
    set(LIBCLIPBOARD_PRIVATE_LIBS ${LIBCLIPBOARD_PRIVATE_LIBS} "-framework Cocoa")
elseif(LIBCLIPBOARD_BUILD_X11)
    include(FindPkgConfig REQUIRED)
    pkg_check_modules(X11 xcb REQUIRED)
    find_package(Threads REQUIRED)

    include_directories(${X11_INCLUDE_DIRS})
    link_directories(${X11_LIBRARY_DIRS})
    set(LIBCLIPBOARD_PRIVATE_LIBS ${LIBCLIPBOARD_PRIVATE_LIBS} ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

# Include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# Configure header
configure_file("include/libclipboard-config.h.in" "include/libclipboard-config.h")

# Source files
set(HEADERS
    include/libclipboard.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/libclipboard-config.h
)

set(SOURCE
    src/clipboard_win32.c
    src/clipboard_x11.c
    src/clipboard_cocoa.c
    src/clipboard_common.c
)

# Set the output folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Make the library
add_library(clipboard ${HEADERS} ${SOURCE})
target_link_libraries(clipboard LINK_PRIVATE ${LIBCLIPBOARD_PRIVATE_LIBS})
if (LIBCLIPBOARD_ADD_SOVERSION)
    # Not by default because my file system doesn't support symlinks
    set_target_properties(clipboard PROPERTIES SOVERSION ${LIBCLIPBOARD_VERSION} VERSION ${LIBCLIPBOARD_VERSION})
endif()

# Testing mode?
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/googletest)
    # gtest does not play well with pthreads and mingw
    if (gtest_disable_pthreads)
        set(LIBCLIPBOARD_USE_PTHREADS_INIT ${CMAKE_USE_PTHREADS_INIT})
        unset(CMAKE_USE_PTHREADS_INIT)
    endif()

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest EXCLUDE_FROM_ALL)
    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

    set(CMAKE_USE_PTHREADS_INIT ${LIBCLIPBOARD_USE_PTHREADS_INIT})
else()

endif()

# Build sample executables
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/samples)

# Pkgconfig
include(FindPkgConfig QUIET)
if (PKGCONFIG_FOUND)
    string(REPLACE "-l" "" LIBCLIPBOARD_PRIVATE_LIBS_LIST "${LIBCLIPBOARD_PRIVATE_LIBS}")
    string(REPLACE ";" " -l" LIBCLIPBOARD_PRIVATE_LIBS_LIST "${LIBCLIPBOARD_PRIVATE_LIBS_LIST}")
    string(STRIP "${LIBCLIPBOARD_PRIVATE_LIBS_LIST}" LIBCLIPBOARD_PRIVATE_LIBS_LIST)
    configure_file("libclipboard.pc.in" "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pkgconfig/libclipboard.pc" @ONLY)
    install(FILES "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/pkgconfig/libclipboard.pc" DESTINATION "lib/pkgconfig")
endif()

# Install options
install(TARGETS clipboard
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)