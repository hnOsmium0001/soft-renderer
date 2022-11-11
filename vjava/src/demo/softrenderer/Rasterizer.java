package demo.softrenderer;

import org.jetbrains.annotations.Nullable;

public class Rasterizer {
    // We have logic that swap x and y, intellij report false positives with those
    @SuppressWarnings("SuspiciousNameCombination")
    public static void drawLine(
            Bitmap colorBuffer,
            @Nullable DepthBuffer depthBuffer,
            float x0, float y0, float z0,
            float x1, float y1, float z1,
            int color) {
        boolean steep = false;
        float tmp;

        if (Math.abs(x0 - x1) < Math.abs(y0 - y1)) {
            // Swap x0 and y0
            tmp = x0;
            x0 = y0;
            y0 = tmp;

            // Swap x1 and y1
            tmp = x1;
            x1 = y1;
            y1 = tmp;

            steep = true;
        }
        if (x0 > x1) {
            // Swap x0 and x1
            tmp = x0;
            x0 = x1;
            x1 = tmp;

            // Swap y0 and y1
            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }

        for (int x = (int) x0; x <= x1; ++x) {
            float t = (x - x0) / (x1 - x0);
            int y = (int) (y0 * (1.0f - t) + y1 * t);
            float z = z0 * (1.0f - t) + z1 * t;
            if (steep) {
                colorBuffer.setPixel(y, x, color);
                if (depthBuffer != null) depthBuffer.setDepth(y, x, z);
            } else {
                colorBuffer.setPixel(x, y, color);
                if (depthBuffer != null) depthBuffer.setDepth(x, y, z);
            }
        }
    }

    public static void drawTriangle() {

    }
}
