#include <iostream>

#include "cherry.hpp"
#include "image.hpp"


int main() {
    Image image("images/originals/chickpeas.gif");
    Image copy(image.w, image.h);

    for (int i = 0; i < image.w; ++ i) {
        for (int j = 0; j < image.h; ++ j) {
            copy.set(i, j, image.pixel(i, j));
        }
    }

    copy.write("images/outputs/chickpeas.png");
    return 0;
}
