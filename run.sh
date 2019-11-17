cmake .
make
./bin/soft_renderer

# For better viewing (some image viewer doesn't support .tga format)
convert ./bin/output.tga ./bin/output.png
