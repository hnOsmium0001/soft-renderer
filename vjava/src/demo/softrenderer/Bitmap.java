package demo.softrenderer;

// Note: packed color stored in ARGB format
//       AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB <-- int, 32 bits
//       \-8bit-/\-8bit-/\-8bit-/\-8bit-/
// Note: stored pixels in a single, packed array should help cache locality
public final class Bitmap {
    public int[] pixels;
    public int width;
    public int height;

    public Bitmap(int width, int height) {
        this.pixels = new int[width * height];
        this.width = width;
        this.height = height;
    }

    public int getPixel(int x, int y) {
        return pixels[y * width + x];
    }

    public void setPixel(int x, int y, int packedColor) {
        pixels[y * width + x] = packedColor;
    }

    public static int getPackedColor(int alpha, int red, int green, int blue) {
        return ((alpha & 0xFF) << 24) |
                ((red & 0xFF) << 16) |
                ((green & 0xFF) << 8) |
                (blue & 0xFF);
    }

    public static int getAlphaComponent(int packedColor) {
        return (packedColor >>> 24) & 0xFF;
    }

    public static int getRedComponent(int packedColor) {
        return (packedColor >>> 16) & 0xFF;
    }

    public static int getGreenComponent(int packedColor) {
        return (packedColor >>> 8) & 0xFF;
    }

    public static int getBlueComponent(int packedColor) {
        return packedColor & 0xFF;
    }
}
