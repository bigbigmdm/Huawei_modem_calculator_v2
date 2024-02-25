#!/bin/bash
if [ "$EUID" -ne 0 ]
  then echo "Please run as root! (sudo ./install.sh)"
  exit
fi
rm -rf build
mkdir build
cd build
cmake ..
make -j4
sudo make install
cd ..
rm -rf build
