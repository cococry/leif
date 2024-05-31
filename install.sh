#!/bin/sh

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
echo "====================="
echo "INSTALLATION FINISHED"
echo "====================="
