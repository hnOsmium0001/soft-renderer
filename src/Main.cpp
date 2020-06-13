#include <argparse/argparse.hpp>
#include "Util.hpp"

auto main(i32 argc, char** argv) -> i32 {
	auto program = argparse::ArgumentParser("GL graphics engine");
	program.add_argument("--workers")
		.help("Number of worker threads for rendering primitives")
		.default_value(4);
	try {
		program.parse_args(argc, argv);
	} catch(const std::runtime_error& error) {
		std::cout << error.what() << "\n";
		std::cout << program;
		return -1;
	}
	return 0;
}
