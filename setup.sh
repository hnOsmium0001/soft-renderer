if [ ! -d "./.tmp/" ]; then
	mkdir ./.tmp/
fi
if [ ! -d "./external/" ]; then
	mkdir external
fi

if [ ! -f ".tmp/3.3.7.tar.gz" ]; then
	# Eigen
	wget http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz -P ./.tmp/
	tar -xf .tmp/3.3.7.tar.gz -C .tmp/

	find ./.tmp/ -type d -name "eigen-eigen-*" -exec mv {}/Eigen external/Eigen \; -quit
fi
