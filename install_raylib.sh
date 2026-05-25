#!/usr/bin/env bash
# One-time setup: install raylib on Linux Mint / Ubuntu.
set -e

echo ">> Installing build deps (sudo)..."
sudo apt update
sudo apt install -y build-essential git cmake \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

TMP="$(mktemp -d)"
echo ">> Cloning raylib into $TMP ..."
git clone --depth 1 https://github.com/raysan5/raylib.git "$TMP/raylib"

cd "$TMP/raylib/src"
echo ">> Building raylib..."
make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED -j"$(nproc)"

echo ">> Installing raylib to /usr/local (sudo)..."
sudo make install PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED

sudo ldconfig

echo ">> Done. raylib installed."
