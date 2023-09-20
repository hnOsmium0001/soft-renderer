package demo.softrenderer;

public class Main {
    public static void main(String[] args) {
        var framebuffer_color = new Bitmap(1024, 768);
        var framebuffer_depth = new DepthBuffer(1024, 768);

        framebuffer_color.clear(0x000000);
        framebuffer_depth.clear(0x000000);
        Rasterizer.drawLine(framebuffer_color, framebuffer_depth, 0, 0, 0, 1024, 768, 1, 0xffffffff);
    }
}
