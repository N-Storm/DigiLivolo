set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

SET (TRIPLET arm-linux-gnueabihf)
SET(CMAKE_C_COMPILER ${TRIPLET}-gcc)

# Build for Cortex-A7 if required
# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a7 -mtune=cortex-a7")

# set path(s) to search for libraries/binaries/headers
# SET (CMAKE_FIND_ROOT_PATH /usr/${TRIPLET} /usr/lib/${TRIPLET} /usr/include /usr/include/${TRIPLET})
# ensure only cross-dirs are searched
# SET (ONLY_CMAKE_FIND_ROOT_PATH TRUE)

# We have cross pkg-config installed instead
SET(PKG_CONFIG_EXECUTABLE ${TRIPLET}-pkg-config)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
