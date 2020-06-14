#include <algorithm>
#include "Render.hpp"

using namespace SRender;

auto FrameBuffer::Merge(std::vector<FrameBuffer> layers) -> tl::expected<FrameBuffer, std::string> {
#ifdef SRENDER_BOUNDS_SAFETY_CHECK
	if (layers.size() == 0) {
		return tl::unexpected("Must provide 1 or more layers to merge");
	}
#endif // SRENDER_BOUNDS_SAFETY_CHECK
	u32 width = layers[0].image.GetWidth();
	u32 height = layers[0].image.GetHeight();
#ifdef SRENDER_BOUNDS_SAFETY_CHECK
	for (auto& layer : layers) {
		if (layer.image.GetWidth() != width
			|| layer.image.GetHeight() != height) {
			return tl::unexpected("Not all layers have the same dimension");
		}
	}
#endif // SRENDER_BOUNDS_SAFETY_CHECK

	auto result = FrameBuffer{width, height};
	for (auto& layer : layers) {
		for (u32 y = 0; y < layer.image.GetHeight(); ++y) {
			for (u32 x = 0; x < layer.image.GetWidth(); ++x) {
				auto depth = layer.GetDepth(x, y);
				if (result.GetDepth(x, y) < depth) {
					result.Set(x, y, layer.Get(x, y));
					result.SetDepth(x, y, depth);
				}
			}
		}
	}
	return result;
}

FrameBuffer::FrameBuffer(u32 width, u32 height)
	: image{TGAImage{width, height, TGAImage::RGB}}
	, depthBuffer(width * height) {}

auto FrameBuffer::Get(u32 x, u32 y) const -> TGAColor {
	return image.Get(x, y);
}

auto FrameBuffer::Set(u32 x, u32 y, TGAColor color) -> bool {
	return image.Set(x, y, color);
}

auto FrameBuffer::GetDepth(u32 x, u32 y) const -> f32 {
	return depthBuffer[x + y * image.GetWidth()];
}

auto FrameBuffer::SetDepth(u32 x, u32 y, f32 z) -> void {
	depthBuffer[x + y * image.GetWidth()] = z;
}

auto FrameBuffer::RenderTriangle(
	const Eigen::Vector3f& v1,
	const Eigen::Vector3f& v2,
	const Eigen::Vector3f& v3,
	const std::function<auto(const Eigen::Vector2f&) -> TGAColor>& frag
) -> void {
	Eigen::Vector2i bbv1 {
		std::max(0.0f, std::min({v1.x(), v2.x(), v3.x()})),
		std::max(0.0f, std::min({v1.y(), v2.y(), v3.y()}))
	};
	Eigen::Vector2i bbv2 {
		std::min(static_cast<f32>(GetWidth()), std::max({v1.x(), v2.x(), v3.x()})),
		std::min(static_cast<f32>(GetHeight()), std::max({v1.y(), v2.y(), v3.y()}))
	};

	for(i32 y = bbv1.y(); y <= bbv2.y(); ++y) {
		for(i32 x = bbv1.x(); x <= bbv2.x(); ++x) {
			auto bc = SRender::Barycentric({static_cast<f32>(x), static_cast<f32>(y), 0.0f}, v1, v2, v3);
			if(bc.x() >= 0 && bc.x() <= 1
				&& bc.y() >= 0 && bc.y() <= 1) {
				f32 bcX =
					v1.x() * bc.x() +
					v2.x() * bc.y() +
					v3.x() * bc.z();
				f32 bcY =
					v1.y() * bc.x() +
					v2.y() * bc.y() +
					v3.y() * bc.z();
				f32 bcZ =
					v1.z() * bc.x() +
					v2.z() * bc.y() +
					v3.z() * bc.z();
				this->Set(x, y, frag({bcX, bcY}));
				this->SetDepth(x, y, bcZ);
			}
		}
	}
}

auto FrameBuffer::RenderTriangles(
	std::span<Eigen::Vector3f> vertices,
	std::span<usize> indices,
	const std::function<auto(const Eigen::Vector2f&) -> TGAColor>& frag
) -> void {
#ifdef SRENDER_BOUNDS_SAFETY_CHECK
	if (indices.size() % 3 != 0) {
		std::cerr << "Indices array provided has a size of non-multiple-of-3";
		return;
	}
#endif // SRENDER_BOUNDS_SAFETY_CHECK

	for (usize i = 0; i < indices.size(); i += 3) {
		this->RenderTriangle(
			vertices[i],
			vertices[i + 1],
			vertices[i + 2],
			frag
		);
	}
}

auto FrameBuffer::RenderLine(
	const Eigen::Vector3f& aIn,
	const Eigen::Vector3f& bIn,
	TGAColor color
) -> void {
	// Copy the vectors for swapping
	auto a = aIn;
	auto b = bIn;

	bool steep = false;
	if (std::abs(a.x() - b.x()) < std::abs(a.y() - b.y())) {
		std::swap(a.x(), a.y());
		std::swap(b.x(), b.y());
		steep = true;
	}
	if(a.x() > b.x()) {
		std::swap(a, b);
	}

	for(u32 x = a.x(); x <= b.x(); ++x) {
		f32 t = (x - a.x()) / static_cast<f32>(b.x() - a.x());
		u32 y = a.y() * (1.0f - t) + b.y() * t;
		f32 z = a.z() * (1.0f - t) + b.z() * t;
		if(steep) {
			this->Set(y, x, color);
			this->SetDepth(y, x, z);
		} else {
			this->Set(x, y, color);
			this->SetDepth(x, y, z);
		}
	}
}

RenderBuffer::RenderBuffer(u32 width, u32 height)
	: FrameBuffer(width, height) {}

auto RenderBuffer::Set(u32 x, u32 y, f32 z, TGAColor color) -> void {
	if (depthTest) {
		auto i = x + y * image.GetWidth();
		if (depthBuffer[i] < z) {
			FrameBuffer::Set(x, y, color);
			if (depthMask) {
				depthBuffer[i] = z;
			}
		}
	} else {
		FrameBuffer::Set(x, y, color);
		if (depthMask) {
			depthBuffer[x + y * image.GetWidth()] = z;
		}
	}
}

template <class V, u32 n>
static auto AnyBarycentric(
	const Eigen::Matrix<V, n, 1>& pt,
	const Eigen::Matrix<V, n, 1>& v1,
	const Eigen::Matrix<V, n, 1>& v2,
	const Eigen::Matrix<V, n, 1>& v3
) -> Eigen::Matrix<V, 3, 1> {
	using Vec = Eigen::Matrix<V, 3, 1>;
	auto u = Vec(v3.x() - v1.x(), v2.x() - v1.x(), v1.x() - pt.x())
		.cross(Vec(v3.y() - v1.y(), v2.y() - v1.y(), v1.y() - pt.y()));

	return std::abs(u.z()) < 1
		? Vec(-1, -1, -1)
		: Vec(1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
}

auto SRender::Barycentric(
	const Eigen::Vector3f& pt,
	const Eigen::Vector3f& v1,
	const Eigen::Vector3f& v2,
	const Eigen::Vector3f& v3
) -> Eigen::Vector3f {
	return AnyBarycentric<f32, 3>(pt, v1, v2, v3);
}

auto SRender::Barycentric(
	const Eigen::Vector2f& pt,
	const Eigen::Vector2f& v1,
	const Eigen::Vector2f& v2,
	const Eigen::Vector2f& v3
) -> Eigen::Vector3f {
	return AnyBarycentric<f32, 2>(pt, v1, v2, v3);
}
