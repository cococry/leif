if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake .. 
make
sudo make install 
