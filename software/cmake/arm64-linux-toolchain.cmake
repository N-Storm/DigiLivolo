set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

SET (TRIPLET aarch64-linux-gnu)
SET(CMAKE_C_COMPILER ${TRIPLET}-gcc)

# We have cross pkg-config installed instead
SET(PKG_CONFIG_EXECUTABLE ${TRIPLET}-pkg-config)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
