#include <iostream>

#include "dft.hpp"


int main() {
    // Set pixels
    Image image1(3, 2), image2(2, 5);
    for (int y = 0; y < 2; ++ y) {
        for (int x = 0; x < 3; ++ x) {
            image1.set(x, y, Pixel(0, 1, 2));
        }
    }
    for (int y = 0; y < 5; ++ y) {
        for (int x = 0; x < 2; ++ x) {
            image2.set(x, y, Pixel(0, 1, 2));
        }
    }

    // DFT
    int dft_w = dft_round(5), dft_h = dft_round(7);
    ComplexPixel *dft_space1, *dft_space2;
    dft_alloc(3, 2, image1.data, dft_w, dft_h, dft_space1);
    dft_alloc(2, 5, image2.data, dft_w, dft_h, dft_space2);
    dft(dft_w, dft_h, dft_space1);
    dft(dft_w, dft_h, dft_space2);
    dft_multiply(dft_w, dft_h, dft_space1, dft_space2);
    dft(dft_w, dft_h, dft_space1, true);

    // Show result
    for (int y = 0, index = 0; y < dft_h; ++ y) {
        for (int x = 0; x < dft_w; ++ x, ++ index) {
            std::cout << dft_space1[index].b << " ";
        }
        std::cout << std::endl;
    }

    // Free
    dft_free(dft_space1);
    dft_free(dft_space2);
}