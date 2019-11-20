cmake .
make
./build/bin/soft_renderer

find ./build -name "*.tga" -exec convert {} {}-convert-output.png \;
