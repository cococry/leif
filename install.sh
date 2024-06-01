#!/bin/sh

get_opengl_version() {
  glxinfo | grep "OpenGL version string" | awk '{print $4}'
}

REQUIRED_VERSION="4.5"
CURRENT_VERSION=$(get_opengl_version)

if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$CURRENT_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
  echo "Your OpenGL version is $CURRENT_VERSION. OpenGL version $REQUIRED_VERSION or higher is required."
  echo "Please update your OpenGL drivers."
  exit 1
fi

echo "OpenGL $CURRENT_VERSION is sufficient."

if ! pkg-config cglm; then
echo "cglm not found on the system"
echo "building cglm"
git clone https://github.com/recp/cglm
cd cglm
mkdir build
cd build
cmake ..
make -j$(nproc)
sudo make install
cd ../../
rm -rf cglm
fi

if ! pkg-config libclipboard; then
echo "libclipboard not found on the system"
echo "building libclipboard"
git clone https://github.com/jtanx/libclipboard
cd libclipboard
cmake .
make -j$(nproc)
sudo make install
cd ..
rm -rf libclipboard
fi

make -j$(nproc) && sudo make install
cp -rf ./.leif ~/

echo "====================="
echo "INSTALLATION FINISHED"
echo "====================="
