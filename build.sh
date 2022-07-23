BUILD_TYPE="${1:-Debug}"
# TODO: Double check which are actually needed for current dependencies
sudo apt install libfreetype-dev
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make
