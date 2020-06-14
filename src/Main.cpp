#include <string>
#include <vector>
#include <span>
#include <future>
#include <Eigen/Dense>
#include <argparse/argparse.hpp>
#include "Util.hpp"
#include "TGAImage.hpp"
#include "Model.hpp"
#include "Render.hpp"

using namespace SRender;

auto Render(u32 width, u32 height, std::span<Eigen::Vector3f> vertices, std::span<usize> indices) -> FrameBuffer {
	auto fr = FrameBuffer{width, height};
	fr.RenderTriangles(
		vertices, indices,
		[](const Eigen::Vector2f& pos) {
			return TGAColor{255, 255, 255, 255}; // TODO
		}
	);
	return fr;
}

auto main(i32 argc, char** argv) -> i32 {
	auto program = argparse::ArgumentParser("GL graphics engine");
	program.add_argument("model")
		.help("The model file to render")
		.required();
	program.add_argument("-o", "--output-file")
		.help("Path to output image file")
		.required();
	program.add_argument("-W", "--resolution-width")
		.help("Output image's width")
		.default_value(1024);
	program.add_argument("-H", "--resolution-height")
		.help("Output image's height")
		.default_value(768);
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

	auto resWidth = program.get<u32>("--resolution-height");
	auto resHeight = program.get<u32>("--resolution-height");
	auto outputPath = program.get<std::string>("--output-file");
	auto modelPath = program.get<std::string>("model");
	auto workersCount = program.get<u32>("--workers");

	auto model = Mesh::ReadOBJAt(modelPath);
	auto splittedTrCount = (model.Indices().size() / 3) / workersCount;
	auto splittedExtras = model.Indices().size() - splittedTrCount * workersCount;

	std::vector<std::future<FrameBuffer>> workerOutputs(workersCount);
	for (usize i = 0; i < workersCount; ++i) {
		auto count = i == workersCount - 1
			? splittedTrCount + splittedExtras
			: splittedTrCount;
		auto start = model.Indices().begin() + splittedTrCount * i;
		auto end = start + count;

		workerOutputs[i] = std::async(
			// Async option
			std::launch::async,
			// Function
			&Render,
			// Parameters
			resWidth, resHeight,
			model.Vertices(), std::span<usize>{start, end}
		);
	}

	// Can't pre-reserve space because FrameBuffer doesn't have a default constructor
	std::vector<FrameBuffer> workerResults;
	for (usize i = 0; i < workersCount; ++i) {
		workerResults.push_back(workerOutputs[i].get());
	}

	FrameBuffer::Merge(workerResults)
		.and_then([&](auto& buf) {
			buf.image.WriteTGAFile(outputPath);
		});

	return 0;
}
