#include <iostream>

#include "cherry.hpp"
#include "image.hpp"


int main() {
    auto image = std::make_shared<Image>("images/originals/chickpeas.gif");

    int w = 1920, h = 1080;
    auto canvas = std::make_shared<Canvas>(w, h);

    for (int y = 0; y < canvas->h; y += 50) {
        for (int x = 0; x < canvas->w; x += 50) {
            auto patch = std::make_shared<Patch>(image, x, y);
            canvas->apply(patch);
        }
    }

    canvas->write("images/outputs/chickpeas.png");

    return 0;
}
