#include <iostream>

#include "cherry.hpp"
#include "image.hpp"


int main() {
    auto image = std::make_shared<Image>("images/originals/strawberries2.gif");

    int w = 512, h = 512;
    auto canvas = std::make_shared<Canvas>(w, h);

    for (int y = 0; y < canvas->h; y += rand() % 20 + 100) {
        for (int x = 0; x < canvas->w; x += rand() % 20 + 100) {
            auto patch = std::make_shared<Patch>(image, x, y);
            canvas->apply(patch);
        }
    }

    canvas->write("images/outputs/strawberries2.png");

    return 0;
}
