#include "Render.hpp"

using namespace SRender;

auto FrameBuffer::Merge(std::vector<FrameBuffer> layers) -> tl::expected<FrameBuffer, std::string> {
	u32 width = layers[0].image.GetWidth();
	u32 height = layers[0].image.GetHeight();
#ifdef SRENDER_BOUNDS_SAFETY_CHECK
	for (auto& layer : layers) {
		if (layer.image.GetWidth() != width
			|| layer.image.GetHeight() != height) {
			return tl::unexpected("Not all layers have the same dimension!");
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

MaskFrameBuffer::MaskFrameBuffer(u32 width, u32 height)
	: FrameBuffer(width, height) {}

auto MaskFrameBuffer::Get(u32 x, u32 y) const -> TGAColor {
	return FrameBuffer::Get(x, y);
}

auto MaskFrameBuffer::Set(u32 x, u32 y, f32 z, TGAColor color) -> void {
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
