#include <iostream>

#include "image.hpp"
#include "placer.hpp"


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: graph_cut <input> <output> <canvas_size>" << std::endl;
        std::cout << "Example: graph_cut peas.png peas_output.png 512x512" << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    std::cout << "Reading image from " << argv[1] << " ..." << std::endl;
    auto texture = std::make_shared<Image>(argv[1]);

    int w, h;
    sscanf(argv[3], "%dx%d", &w, &h);
    std::cout << "Making " << w << "x" << h << " canvas ..." << std::endl;
    auto canvas = std::make_shared<Canvas>(w, h);

    std::cout << "Begin to apply patches on canvas:" << std::endl;
    Placer::init(canvas, texture);

    // Refine
    std::cout << "Begin to refine:" << std::endl;
    for (int i = 0; i < 100; ++ i) {
        Placer::entire_matching(canvas, texture);
    }

    std::cout << "Writing result into " << argv[2] << " ..." << std::endl;
    canvas->write(argv[2]);

    return 0;
}
