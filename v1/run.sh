if [ ! -d "build" ]; then
	mkdir build
	cd build
	conan install .. -b missing
	cmake .. -G Ninja
fi

ninja
./bin/soft_renderer

find ./build -name "*.tga" -exec convert {} {}-convert-output.png \;
