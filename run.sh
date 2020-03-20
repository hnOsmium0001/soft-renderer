mkdir build
cd build

cmake ..
make
./bin/soft_renderer

find ./build -name "*.tga" -exec convert {} {}-convert-output.png \;
