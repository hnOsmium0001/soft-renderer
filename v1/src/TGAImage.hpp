#pragma once

#include <fstream>
#include "Util.hpp"

namespace SRender {

#pragma pack(push,1)
struct TGAHeader {
	u8 idlength;
	u8 colormaptype;
	u8 datatypecode;
	i16 colormaporigin;
	i16 colormaplength;
	u8 colormapdepth;
	i16 x_origin;
	i16 y_origin;
	i16 width;
	i16 height;
	u8 bitsperpixel;
	u8 imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
	union {
		struct {
			u8 b;
			u8 g;
			u8 r;
			u8 a;
		};
		u8 raw[4];
		u32 val;
	};
	i32 bytespp;

	TGAColor()
		: val{0}, bytespp{1} {}
	
	TGAColor(u8 r, u8 g, u8 b, u8 a)
		: r{r}, g{g}, b{b}, a{a} {}
	
	TGAColor(u32 val, i32 bytespp)
		: val{val}, bytespp{bytespp} {}

	TGAColor(const u8* p, i32 bytespp)
		: val{0}, bytespp{bytespp} {
		for (usize i = 0; i < bytespp; ++i) {
			raw[i] = p[i];
		}
	}
	
	TGAColor(const TGAColor&) = default;
	TGAColor& operator=(const TGAColor&) = default;
	TGAColor(TGAColor&&) = default;
	TGAColor& operator=(TGAColor&&) = default;
};

class TGAImage {
protected:
	u8* data;
	u32 width;
	u32 height;
	i32 bytespp;
	
	auto LoadRLEData(std::ifstream& in) -> bool;
	auto UnloadRLEData(std::ofstream& out) -> bool;
public:
	enum Format {
		GRAYSCALE = 1,
		RGB = 3,
		RGBA = 4
	};
	
	TGAImage();
	TGAImage(u32 w, u32 h, i32 bpp);
	~TGAImage();

	TGAImage(const TGAImage& img);
	TGAImage& operator=(const TGAImage& img);
	TGAImage(TGAImage&& img);
	TGAImage& operator=(TGAImage&& img);

	auto ReadTGAFile(std::string_view filename) -> bool;
	auto WriteTGAFile(std::string_view filename, bool rle = true) -> bool;
	auto Get(u32 x, u32 y) const -> TGAColor;
	auto Set(u32 x, u32 y, TGAColor c) -> bool;
	auto FlipHorizontally() -> bool;
	auto FlipVertically() -> bool;
	auto Scale(i32 w, i32 h) -> bool;
	auto Clear() -> void;

	auto GetWidth() const -> u32 { return width; }
	auto GetHeight() const-> u32 { return height; }
	auto GetBytesPP() const -> i32 { return bytespp; }
	auto Buffer() const -> u8* { return data; }
};

} // namespace SRender
