cmake .
make
./bin/soft_renderer

# For better viewing (some image viewer doesn't support .tga format)
# if [ -f ./bin/output.tga ]; then
#   convert ./bin/output.tga ./bin/output.png
# fi
# if [ -f ./bin/zdump.tga ]; then
#   convert ./bin/zdump.tga ./bin/zdump.png
# fi

find ./bin -name "*.tga" -exec convert {} {}-convert-output.png \;
