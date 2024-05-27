git clone https://github.com/recp/cglm
cd cglm
mkdir build
cd build
cmake ..
make
sudo make install
cd ../../
rm -rf cglm
git clone https://github.com/jtanx/libclipboard
cd libclipboard
cmake .
make -j4
sudo make install
cd ..
rm -rf libclipboard
make && sudo make install
echo "====================="
echo "INSTALLATION FINISHED"
echo "====================="
