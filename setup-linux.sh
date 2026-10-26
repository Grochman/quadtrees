#!/usr/bin/env bash
# Download and extract a local toolchain into .toolchain inside this repo.
# sudo is not required.
#
# Default OpenCL runtime is intel-opencl-icd (Intel Xe / Gen12 and newer).
# Pass --legacy for intel-opencl-icd-legacy (Gen8–Gen11, including Kaby Lake
# UHD 620). Install only one of them. The program uses the first OpenCL platform.
#
# Usage: setup-linux.sh [--legacy]
set -euo pipefail

LEGACY=0
if [[ "${1:-}" == "--legacy" ]]; then
  LEGACY=1
elif [[ -n "${1:-}" ]]; then
  echo "usage: $0 [--legacy]" >&2
  exit 1
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX="$ROOT/.toolchain"
DEBDIR="$PREFIX/debs"
mkdir -p "$DEBDIR"

if [[ "$LEGACY" == 1 ]]; then
  OPENCL_PKGS=(libigc1 libigdfcl1 intel-opencl-icd-legacy)
  rm -f "$DEBDIR"/libigc2_*.deb "$DEBDIR"/libigdfcl2_*.deb "$DEBDIR"/intel-opencl-icd_*.deb
else
  OPENCL_PKGS=(libigc2 libigdfcl2 intel-opencl-icd)
  rm -f "$DEBDIR"/libigc1_*.deb "$DEBDIR"/libigdfcl1_*.deb "$DEBDIR"/intel-opencl-icd-legacy_*.deb
fi

cd "$DEBDIR"
apt-get download \
  clinfo cmake-data libjsoncpp26 librhash1 cmake \
  "${OPENCL_PKGS[@]}" \
  libglx-dev libgl-dev libpkgconf7 \
  libsfml-system3.0 libsfml-audio3.0 libsfml-window3.0 libstb0t64 \
  libsfml-graphics3.0 libsfml-network3.0 \
  ninja-build pkgconf-bin pkgconf pkg-config libsfml-dev \
  opencl-c-headers opencl-clhpp-headers opencl-headers ocl-icd-opencl-dev

rm -rf "$PREFIX/etc/OpenCL/vendors"
for deb in "$DEBDIR"/*.deb; do
  dpkg-deb -x "$deb" "$PREFIX"
done

# Dev package symlink points at a file that exists only in the system loader package.
ln -sfn /usr/lib/x86_64-linux-gnu/libOpenCL.so.1 \
  "$PREFIX/usr/lib/x86_64-linux-gnu/libOpenCL.so"

# Packaged ICD files name a path under /usr. Point them at the extracted library.
for icd in "$PREFIX/etc/OpenCL/vendors/"*.icd; do
  line="$(head -n 1 "$icd")"
  if [[ "$line" == /usr/* ]]; then
    printf '%s\n' "$PREFIX$line" > "$icd"
  fi
done

echo "Toolchain ready in $PREFIX"
if [[ "$LEGACY" == 1 ]]; then
  echo "OpenCL runtime: intel-opencl-icd-legacy"
else
  echo "OpenCL runtime: intel-opencl-icd"
fi
echo "Next: source env-linux.sh && cmake -S . -B build -G Ninja && cmake --build build"
