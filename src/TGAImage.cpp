#include <iostream>
#include <cmath>
#include <cstring>
#include <ctime>
#include "TGAImage.hpp"

using namespace SRender;

TGAImage::TGAImage()
	: data{nullptr}, width{0}, height{0}, bytespp{0} {}

TGAImage::TGAImage(u32 w, u32 h, i32 bpp)
	: data{nullptr}
	, width{w}, height{h}
	, bytespp{bpp} {
	u64 nbytes = width * height * bytespp;
	data = new u8[nbytes];
	memset(data, 0, nbytes);
}

TGAImage::~TGAImage() {
	if (data) {
		delete[] data;
	}
}

TGAImage::TGAImage(const TGAImage& img)
	: width{img.width}, height{img.height}
	, bytespp{img.bytespp} {
	u64 nbytes = width * height * bytespp;
	data = new u8[nbytes];
	memcpy(data, img.data, nbytes);
}

auto TGAImage::operator=(const TGAImage& img) -> TGAImage& {
	if (data) {
		delete[] data;
	}
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	u64 nbytes = width * height * bytespp;
	data = new u8[nbytes];
	memcpy(data, img.data, nbytes);
	return *this;
}

TGAImage::TGAImage(TGAImage&& img)
	: width{img.width}, height{img.height}
	, bytespp{img.bytespp}
	, data{img.data} {
	img.data = nullptr;
}

auto TGAImage::operator=(TGAImage&& img) -> TGAImage& {
	if (data) {
		delete[] data;
	}
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	data = img.data;
	img.data = nullptr;
	return *this;
}

auto TGAImage::ReadTGAFile(std::string_view filename) -> bool {
	if (data) {
		delete[] data;
	}

	data = nullptr;
	std::ifstream in;
	in.open(filename.data(), std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		in.close();
		return false;
	}
	TGAHeader header;
	in.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!in.good()) {
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}
	width = header.width;
	height = header.height;
	bytespp = header.bitsperpixel >> 3;
	if (width <= 0 || height <= 0 ||
		(bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
		in.close();
		std::cerr << "bad bpp (or width/height) value\n";
		return false;
	}

	u64 nbytes = bytespp * width * height;
	data = new u8[nbytes];
	if (3 == header.datatypecode || 2 == header.datatypecode) {
		in.read(reinterpret_cast<char*>(data), nbytes);
		if (!in.good()) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else if (10 == header.datatypecode || 11 == header.datatypecode) {
		if (!LoadRLEData(in)) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else {
		in.close();
		std::cerr << "unknown file format " << static_cast<i32>(header.datatypecode) << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20)) {
		FlipVertically();
	}
	if (header.imagedescriptor & 0x10) {
		FlipHorizontally();
	}
	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
	in.close();
	return true;
}

auto TGAImage::LoadRLEData(std::ifstream& in) -> bool {
	u64 pixelcount = width * height;
	u64 currentpixel = 0;
	u64 currentbyte = 0;
	TGAColor colorbuffer;
	do {
		u8 chunkheader = 0;
		chunkheader = in.get();
		if (!in.good()) {
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
		if (chunkheader < 128) {
			chunkheader++;
			for (i32 i = 0; i < chunkheader; i++) {
				in.read(reinterpret_cast<char*>(colorbuffer.raw), bytespp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the header\n";
					return false;
				}
				for (i32 t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		} else {
			chunkheader -= 127;
			in.read(reinterpret_cast<char*>(colorbuffer.raw), bytespp);
			if (!in.good()) {
				std::cerr << "an error occured while reading the header\n";
				return false;
			}
			for (i32 i = 0; i < chunkheader; i++) {
				for (i32 t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

auto TGAImage::WriteTGAFile(std::string_view filename, bool rle) -> bool {
	u8 developerAreaRef[4] = {0, 0, 0, 0};
	u8 extensionAreaRef[4] = {0, 0, 0, 0};
	u8 footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O',
		'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
	std::ofstream out;
	out.open(filename.data(), std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		out.close();
		return false;
	}
	TGAHeader header;
	memset(reinterpret_cast<void*>(&header), 0, sizeof(header));
	header.bitsperpixel = bytespp << 3;
	header.width = width;
	header.height = height;
	header.datatypecode = bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2);
	header.imagedescriptor = 0x20; // top-left origin
	out.write(reinterpret_cast<char*>(&header), sizeof(header));
	if (!out.good()) {
		out.close();
		std::cerr << "can't dump the tga file\n";
		return false;
	}
	if (!rle) {
		out.write(reinterpret_cast<char*>(data), width * height * bytespp);
		if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	} else {
		if (!UnloadRLEData(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write(reinterpret_cast<char*>(developerAreaRef), sizeof(developerAreaRef));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write(reinterpret_cast<char*>(extensionAreaRef), sizeof(extensionAreaRef));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write(reinterpret_cast<char*>(footer), sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

auto TGAImage::UnloadRLEData(std::ofstream &out) -> bool {
	const u8 max_chunk_length = 128;
	u64 npixels = width * height;
	u64 curpix = 0;
	while (curpix < npixels) {
		u64 chunkstart = curpix * bytespp;
		u64 curbyte = curpix * bytespp;
		u8 run_length = 1;
		auto raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length) {
			auto succ_eq = true;
			for (u32 t = 0; succ_eq && t < bytespp; t++) {
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write(reinterpret_cast<char*>(data + chunkstart),
							(raw ? run_length * bytespp : bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

auto TGAImage::Get(u32 x, u32 y) const -> TGAColor {
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return TGAColor{};
	}
	return TGAColor{data + (x + y * width) * bytespp, bytespp};
}

auto TGAImage::Set(u32 x, u32 y, TGAColor c) -> bool {
	if (!data || x < 0 || y < 0 || x >= width || y >= height) {
		return false;
	}
	memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
	return true;
}

auto TGAImage::FlipHorizontally() -> bool {
	if (!data) {
		return false;
	}
	i32 half = width >> 1;
	for (i32 i = 0; i < half; i++) {
		for (i32 j = 0; j < height; j++) {
			auto c1 = Get(i, j);
			auto c2 = Get(width - 1 - i, j);
			Set(i, j, c2);
			Set(width - 1 - i, j, c1);
		}
	}
	return true;
}

auto TGAImage::FlipVertically() -> bool {
	if (!data) {
		return false;
	}
	u64 bytes_per_line = width * bytespp;
	u8* line = new unsigned char[bytes_per_line];
	i32 half = height >> 1;
	for (i32 j = 0; j < half; j++) {
		u64 l1 = j * bytes_per_line;
		u64 l2 = (height - 1 - j) * bytes_per_line;
		memmove(reinterpret_cast<void*>(line), reinterpret_cast<void*>(data + l1), bytes_per_line);
		memmove(reinterpret_cast<void*>(data + l1), reinterpret_cast<void*>(data + l2), bytes_per_line);
		memmove(reinterpret_cast<void*>(data + l2), reinterpret_cast<void*>(line), bytes_per_line);
	}
	delete[] line;
	return true;
}

auto TGAImage::Scale(i32 w, i32 h) -> bool {
	if (w <= 0 || h <= 0 || !data) {
		return false;
	}
	u8* tdata = new u8[w * h * bytespp];
	i32 nscanline = 0;
	i32 oscanline = 0;
	i32 erry = 0;
	u64 nlinebytes = w * bytespp;
	u64 olinebytes = width * bytespp;
	for (i32 j = 0; j < height; j++) {
		i32 errx = width - w;
		i32 nx = -bytespp;
		i32 ox = -bytespp;
		for (i32 i = 0; i < width; i++) {
			ox += bytespp;
			errx += w;
			while (errx >= static_cast<i32>(width)) {
				errx -= width;
				nx += bytespp;
				memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry >= static_cast<i32>(height)) {
			if (erry >= static_cast<i32>(height) << 1) // it means we jump over a scanline
				memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete[] data;
	data = tdata;
	width = w;
	height = h;
	return true;
}

auto TGAImage::Clear() -> void {
	memset(reinterpret_cast<void*>(data), 0, width * height * bytespp);
}
