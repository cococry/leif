# libclipboard testing suite

# Source files
set (SOURCE
     test_basics.cpp
     test_custom_allocators.cpp
)
set (HEADERS
    libclipboard-test-private.h
)

# Add the target
add_executable (run-tests ${HEADERS} ${SOURCE})
add_executable (run-smoke1 smoke_test1.c)

# Link it to gtest
target_link_libraries(run-tests LINK_PRIVATE gtest gtest_main)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(run-tests PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

# Link it with libclipboard
target_link_libraries (run-tests LINK_PUBLIC clipboard)
target_link_libraries (run-smoke1 LINK_PUBLIC clipboard)

# For `make test`
add_test(NAME libclipboard-testing
         COMMAND run-tests --gtest_color=yes
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME libclipboard-smoke1
         COMMAND run-smoke1
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(libclipboard-smoke1 PROPERTIES TIMEOUT 180)

# Sigh
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  ARGS "-V"
                  DEPENDS run-tests run-smoke1)