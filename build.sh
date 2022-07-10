BUILD_TYPE="${1:-Debug}"
sudo apt install libfreetype-dev libopenal-dev libvorbis-dev libflac-dev
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make
