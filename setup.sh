if [ ! -d "./tmp/" ]; then
  mkdir ./tmp/
fi

if [ ! -f "tmp/3.3.7.tar.gz" ]; then
  # Eigen
  wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz -P ./tmp/
  tar -xf tmp/3.3.7.tar.gz -C tmp/

  find ./tmp/ -type d -name "eigen-eigen-*" -exec mv {}/Eigen external/ \; -quit
  mv external/Eigen external/eigen
fi
# if [ ! -f "tmp/v1.1.3.tar.gz" ]; then
#   # variant
#   wget https://github.com/mapbox/variant/archive/master.zip -P ./tmp/
#   unzip tmp/master.zip -d tmp/

#   find ./tmp/ -type d -name "variant-*" -exec mv {}/include/ external/ \; -quit
# mv external/include external/variant
# fi
