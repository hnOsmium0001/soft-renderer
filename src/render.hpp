#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <Eigen/Dense>
#include "tgaimage.hpp"

namespace SRender {

	class FrameBuffer {
	private:
		TGAImage image_;
		std::vector<int> zBuffer_;

		bool enableDepthTest = true;
		bool enableDepthWrite = true;

	public:
		FrameBuffer(int width, int height, int defaultZ=0);
		void Write(const std::string path);
		int Test(int x, int y, int z);
		int Test(const Eigen::Vector3i& v);
		void Set(int x, int y, int z, TGAColor color);
		void Set(const Eigen::Vector3i& v, TGAColor color);
		void SetCb(int x, int y, int z, std::function<TGAColor()>&& cb);
		void SetCb(const Eigen::Vector3i& v, std::function<TGAColor()>&& cb);

		void SetDepthTest(bool enable) { enableDepthTest = enable; }
		void setDepthWrite(bool enable) { enableDepthWrite = enable; }

		// Getters
		int width() { return image_.get_width(); }
		int height() { return image_.get_width(); }
		TGAImage& image() { return image_; }
		const TGAImage& image() const { return image_; }
		std::vector<int>& zBuffer() { return zBuffer_; }
		const std::vector<int>& zBuffer() const { return zBuffer_; }
	};

	// Render pipline for 2D and 3D rendering

	class Pipeline {
	public:
		using VCallback = std::function<auto(const Eigen::Vector3f&) -> Eigen::Vector3i>;
		using FCallback = std::function<auto(const Eigen::Vector3i&, std::optional<const Eigen::Vector3f>) -> TGAColor>;
		using VerticesVector = const std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>&;
		using RefVerticesVector = const std::vector<const Eigen::Vector3f&>;
		using IndicesVector = const std::vector<int>&;

	private:
		VCallback vsh_;
		FCallback fsh_;

	public:
		Pipeline() = default;
		void BindShader(VCallback vsh);
		void BindShader(FCallback fsh);
		void BindShaders(VCallback vsh, FCallback fsh);
		void DrawLines(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
		void DrawTriangles(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
		void DrawTriangleStrip(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);
		void DrawPolygon(VerticesVector verts, IndicesVector indices, FrameBuffer& frame);

		VCallback& vsh() { return vsh_; }
		const VCallback& vsh() const { return vsh_; }
		FCallback& fsh() { return fsh_; }
		const FCallback& fsh() const { return fsh_; }
	};

	// 3D specific rendering

	class Camera {
	private:
		Eigen::Matrix4f view_;
		Eigen::Matrix4f projection_;
		Eigen::Matrix4f viewport_;

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		Camera();
		void LookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& up, const Eigen::Vector3f& center);
		void Viewport(int x, int y, int w, int h);
		void Perspective(float fov, float aspect, float near, float far);
		Eigen::Vector3f Transform(const Eigen::Vector3f& pt);

		Eigen::Matrix4f& view() { return view_; }
		const Eigen::Matrix4f& view() const { return view_; }
		Eigen::Matrix4f& projection() { return projection_; }
		const Eigen::Matrix4f& projection() const { return projection_; }
		Eigen::Matrix4f& viewport() { return viewport_; }
		const Eigen::Matrix4f& viewport() const { return viewport_; }
	};

	// Misc utilities

	template <class N>
	N Map(N value, N fromMin, N fromMax, N toMin, N toMax);

	template <class V>
	Eigen::Matrix<V, 3, 1> Barycentric(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

	template <class V>
	bool PtInTriangle(const Eigen::Matrix<V, 3, 1>& pt, const Eigen::Matrix<V, 3, 1>& v1, const Eigen::Matrix<V, 3, 1>& v2, const Eigen::Matrix<V, 3, 1>& v3);

	Eigen::Vector4f RegToAffine(const Eigen::Vector3f& v);
	Eigen::Vector3f AffineToReg(const Eigen::Vector4f& v);

	namespace Debug {

		void DumpZBufferConsole(FrameBuffer& target);
		void DumpZBufferTGASimple(FrameBuffer& target);
		void DumpZBufferTGAFull(FrameBuffer& target);

	}

	namespace Raster {
		void DrawLine(
			const Eigen::Vector3i& v1In,
			const Eigen::Vector3i& v2In,
			SRender::FrameBuffer& frame,
			TGAColor color
		);

		void DrawTriangle(
			const Eigen::Vector3i& v1,
			const Eigen::Vector3i& v2,
			const Eigen::Vector3i& v3,
			SRender::FrameBuffer& frame,
			TGAColor color
		);

		void DrawPolygon(
			const std::vector<Eigen::Vector3i>& verts,
			SRender::FrameBuffer& frame,
			TGAColor color
		);
	}
}
