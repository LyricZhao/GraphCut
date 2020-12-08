#pragma once

#include "image.hpp"


/// Round length to 2^n
int dft_round(int x) {
    int len = 1;
    while (len < x) {
        len *= 2;
    }
    return len;
}


/// Get low-bit number
int dft_lowbit(int x) {
    return x & (-x);
}


/// Allocate DFT working space
void dft_alloc(int w, int h, Pixel *pixels, int dft_w, int dft_h, ComplexPixel* &dft_space) {
    dft_space = static_cast<ComplexPixel*> (std::malloc(dft_w * dft_h * sizeof(ComplexPixel)));
    std::fill(dft_space, dft_space + dft_w * dft_h, ComplexPixel());
    for (int i = 0, index = 0; i < h; ++ i) {
        for (int j = 0; j < w; ++ j, ++ index) {
            dft_space[i * dft_w + j] = to_complex_pixel(pixels[index]);
        }
    }
}


/// Run DFT and IDFT
void dft(int dft_w, int dft_h, ComplexPixel* pixels, bool inverse=false) {
    // Allocate space
    double coefficient = inverse ? -1 : 1;
    assert(dft_w > 0 and dft_h > 0 and dft_w == dft_lowbit(dft_w) and dft_h == dft_lowbit(dft_h));

    // Butterfly changes by w
    for (int row = 0; row < dft_h; ++ row) {
        ComplexPixel *base = pixels + row * dft_w;
        for(int i = 0, j = 0; i < dft_w; ++ i){
            if (i > j) {
                std::swap(base[i], base[j]);
            }
            for (int t = dft_w / 2; (j ^= t) < t; t /= 2);
        }
    }

    // Butterfly changes by h
    for (int col = 0; col < dft_w; ++ col) {
        ComplexPixel *base = pixels + col;
        for(int i = 0, j = 0; i < dft_h; ++ i){
            if (i > j) {
                std::swap(base[i * dft_w], base[j * dft_w]);
            }
            for (int t = dft_h / 2; (j ^= t) < t; t /= 2);
        }
    }

    // DFT by w
    for (int row = 0; row < dft_h; ++ row) {
        ComplexPixel *base = pixels + row * dft_w;
        ComplexPixel wn, w, t, u;
        for(int m = 2; m <= dft_w; m *= 2){
            wn = ComplexPixel(cos(2.0 * M_PI / m), coefficient * sin(2.0 * M_PI / m));
            for (int i = 0 ; i < dft_w; i += m) {
                w = ComplexPixel(1, 0);
                for (int k = 0, p = m / 2; k < p; ++ k, w = w * wn) {
                    t = w * base[i + k + m / 2];
                    u = base[i + k];
                    base[i + k] = u + t;
                    base[i + k + m / 2] = u - t;
                }
            }
        }
    }

    // DFT by h
    for (int col = 0; col < dft_w; ++ col) {
        ComplexPixel *base = pixels + col;
        ComplexPixel wn, w, t, u;
        for(int m = 2; m <= dft_h; m *= 2){
            wn = ComplexPixel(cos(2.0 * M_PI / m), coefficient * sin(2.0 * M_PI / m));
            for (int i = 0 ; i < dft_h; i += m) {
                w = ComplexPixel(1, 0);
                for (int k = 0, p = m / 2; k < p; ++ k, w = w * wn) {
                    t = w * base[(i + k + m / 2) * dft_w];
                    u = base[(i + k) * dft_w];
                    base[(i + k) * dft_w] = u + t;
                    base[(i + k + m / 2) * dft_w] = u - t;
                }
            }
        }
    }

    // Inverse
    if (inverse) {
        double inv = 1.0 / (dft_w * dft_h);
        for (int i = 0; i < dft_w * dft_h; ++ i) {
            pixels[i] = pixels[i] * inv;
        }
    }
}


/// Multiply DFT result 2 into result 1
void dft_multiply(int dft_w, int dft_h, ComplexPixel* dft_result1, ComplexPixel* dft_result2) {
    assert(dft_w > 0 and dft_h > 0 and dft_w == dft_lowbit(dft_w) and dft_h == dft_lowbit(dft_h));
    for (int i = 0; i < dft_w * dft_h; ++ i) {
        dft_result1[i] = dft_result1[i] * dft_result2[i];
    }
}


/// Free DFT result memory
void dft_free(ComplexPixel* dft_result) {
    assert(dft_result);
    std::free(dft_result);
}