#!/bin/bash
# Part of the DigiLivolo control software.
# https://github.com/N-Storm/DigiLivolo/
#
# Builds project from sources with a cmake.
# If called with an argument "full" (build.sh full), does a full rebuild.
#
# SPDX-License-Identifier: GPL-3.0-or-later

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Working dir: ${SCRIPT_DIR}"
pushd ${SCRIPT_DIR}

if [[ ! -z "$1" ]] && [[ "$1" == "full" ]]; then
    rm -r ${SCRIPT_DIR}/build
fi

# cmake -DUSE_SYSTEM_HIDAPI=false -B ${SCRIPT_DIR}/build -G 'Unix Makefiles' ${SCRIPT_DIR}
# cmake -DUSE_SYSTEM_HIDAPI=false -B ${SCRIPT_DIR}/build -G 'Ninja' -Wno-dev ${SCRIPT_DIR}
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_HIDAPI=false -Wno-dev -B ${SCRIPT_DIR}/build ${SCRIPT_DIR}

cmake --build ${SCRIPT_DIR}/build

popd
