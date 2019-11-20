if [ ! -d "./tmp/" ]; then
  mkdir ./tmp/
else
  rm -rf ./tmp/*
fi

# Download libraries
wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz -P ./tmp/

tar -xf tmp/*.tar.gz -C tmp/

# Process libraries
find ./tmp/ -type d -name "eigen-eigen-*" -exec mv {}/Eigen external/ \; -quit
