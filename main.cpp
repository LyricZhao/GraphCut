#include <iostream>

#include "cherry.hpp"
#include "image.hpp"


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: graph_cut <input> <output> <canvas_size>" << std::endl;
        std::cout << "Example: graph_cut peas.png peas_output.png 512x512" << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    std::cout << "Reading image from " << argv[1] << " ..." << std::endl;
    auto image = std::make_shared<Image>(argv[1]);

    int w, h;
    sscanf(argv[3], "%dx%d", &w, &h);
    std::cout << "Making " << w << "x" << h << " canvas ..." << std::endl;
    auto canvas = std::make_shared<Canvas>(w, h);

    std::cout << "Begin to apply patches on canvas:" << std::endl;
    for (int y = 0; y < canvas->h; y += rand() % (image->h / 2) + image->h / 3) {
        for (int x = 0; x < canvas->w; x += rand() % (image->w / 2) + image->w / 3) {
            std::cout << " > Applying a new patch at (" << x << ", " << y << ") ..." << std::endl;
            auto patch = std::make_shared<Patch>(image, x, y);
            canvas->apply(patch);
        }
    }

    std::cout << "Writing result into " << argv[2] << " ..." << std::endl;
    canvas->write(argv[2]);

    return 0;
}
