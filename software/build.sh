#!/bin/bash

# Builds project with cmake.
# If called with any argument, does a full rebuild by moving a build dir to
# build.old (deleting build.old if exits).

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Working dir: ${SCRIPT_DIR}"

if [[ ! -z "$1" ]]; then
  rm -rf ${SCRIPT_DIR}/build.old
  mv ${SCRIPT_DIR}/build ${SCRIPT_DIR}/build.old
fi

mkdir ${SCRIPT_DIR}/build 2>/dev/null
pushd .
cd ${SCRIPT_DIR}/build

# cmake -DUSE_SYSTEM_HIDAPI=false -G 'Unix Makefiles' ..
cmake -DUSE_SYSTEM_HIDAPI=false -G 'Ninja' -Wno-dev ..
cmake --build .

popd
