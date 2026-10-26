# Local toolchain extracted into .toolchain (see setup-linux.sh).
# Source this file before configuring, building, or running without a system install:
#   source env-linux.sh

_QUADTREES_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_TOOLCHAIN="$_QUADTREES_ROOT/.toolchain"

export PATH="$_TOOLCHAIN/usr/bin:${PATH}"
export LD_LIBRARY_PATH="$_TOOLCHAIN/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export LIBRARY_PATH="$_TOOLCHAIN/usr/lib/x86_64-linux-gnu${LIBRARY_PATH:+:$LIBRARY_PATH}"
export CPATH="$_TOOLCHAIN/usr/include${CPATH:+:$CPATH}"
export PKG_CONFIG_PATH="$_TOOLCHAIN/usr/lib/x86_64-linux-gnu/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
export CMAKE_PREFIX_PATH="$_TOOLCHAIN/usr${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
export OCL_ICD_VENDORS="$_TOOLCHAIN/etc/OpenCL/vendors"

unset _QUADTREES_ROOT _TOOLCHAIN
