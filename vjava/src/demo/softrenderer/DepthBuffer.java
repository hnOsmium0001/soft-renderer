package demo.softrenderer;

import java.util.Arrays;

public final class DepthBuffer {
    float[] storage;
    int width;
    int height;

    public DepthBuffer(int width, int height) {
        this.storage = new float[width * height];
        this.width = width;
        this.height = height;
    }

    public float getDepth(int x, int y) {
        return storage[y * width + x];
    }

    public void setDepth(int x, int y, float depth) {
        storage[y * width + x] = depth;
    }

    public void clear(float depth) {
        Arrays.fill(storage, depth);
    }
}
