# libclipboard sample executables

# Set executables
add_executable(clip_sample1 clipboard_sample1.c)

# Link it with libclipboard
target_link_libraries(clip_sample1 LINK_PUBLIC clipboard)
