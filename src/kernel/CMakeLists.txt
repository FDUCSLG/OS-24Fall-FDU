# Glob all .c and .S files in the current directory
file(GLOB kernel_sources CONFIGURE_DEPENDS "*.c" "*.S")

# Create a static library from the globbed sources
add_library(kernelx STATIC ${kernel_sources})

# Ensure that assembly files are handled correctly
set_property(SOURCE ${kernel_sources} PROPERTY LANGUAGE C)