#!/bin/bash

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew is not installed. Please install it first."
    echo "You can install it by running:"
    echo '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
    exit 1
fi

# Install required packages
brew install exiftool jq glfw cglm 2>/dev/null

# Check Mac's graphics capabilities
METAL_SUPPORT=$(system_profiler SPDisplaysDataType | grep "Metal: Supported")
if [[ -n "$METAL_SUPPORT" ]]; then
    echo "Your Mac supports Metal, which should be sufficient for graphics operations."
else
    echo "Metal support not detected. This project may face compatibility issues on your system."
    echo "Proceeding with installation anyway..."
fi

# Build and install cglm if not already installed
if ! pkg-config --exists cglm; then
    echo "cglm not found on the system"
    echo "building cglm"
    git clone https://github.com/recp/cglm
    cd cglm
    mkdir build
    cd build
    cmake ..
    make -j$(sysctl -n hw.ncpu)
    sudo make install
    cd ../../
    rm -rf cglm
fi

# Build and install libclipboard
if ! pkg-config --exists libclipboard; then
    echo "libclipboard not found on the system"
    echo "building libclipboard"
    git clone https://github.com/jtanx/libclipboard
    cd libclipboard
    cmake .
    make -j$(sysctl -n hw.ncpu)
    sudo make install
    cd ..
    rm -rf libclipboard
fi

# Modify the Makefile to use Homebrew's include and lib directories
sed -i '' 's/-I\/usr\/local\/include/-I\/opt\/homebrew\/include/g' Makefile
sed -i '' 's/-L\/usr\/local\/lib/-L\/opt\/homebrew\/lib/g' Makefile

# Build Leif
make -j$(sysctl -n hw.ncpu) && sudo make install
cp -rf ./.leif ~/

echo "====================="
echo "INSTALLATION FINISHED"
echo "====================="
echo "If you encounter any issues running Leif, please contact the project maintainers"
echo "for guidance on macOS compatibility."