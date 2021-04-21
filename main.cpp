#include <iostream>
#include <sys/time.h>   // for timing
#include <iomanip>      // for print time

#include "image.hpp"
#include "placer.hpp"


int main(int argc, char* argv[]) {
    // prompt
    if (argc != 4) {
        std::cout << "Usage: graph_cut <input> <output> <canvas_size>" << std::endl;
        std::cout << "Example: graph_cut peas.png peas_output.png 512x512" << std::endl;
        std::exit(EXIT_SUCCESS);
    }
    // read image
    std::cout << "Reading image from " << argv[1] << " ..." << std::endl;
    auto texture = std::make_shared<Image>(argv[1]);
    
    // initiate canvas
    int w, h;
    sscanf(argv[3], "%dx%d", &w, &h);
    std::cout << "Making " << w << "x" << h << " canvas ..." << std::endl;
    auto canvas = std::make_shared<Canvas>(w, h);

    // initiate timing
    struct timeval start, end;
    double placerTime, refineTime;
    
    
    // draw on canvas
    std::cout << "Begin to apply patches on canvas:" << std::endl;
    gettimeofday(&start, NULL); // start timer
    Placer::init(canvas, texture);
    gettimeofday(&end, NULL);   // stop timer
    // get darwing time
    placerTime = (end.tv_sec - start.tv_sec) * 1e6;
    placerTime = (placerTime + (end.tv_sec - start.tv_sec)) * 1e6;
    
    
    // Refine
    std::cout << "Begin to refine:" << std::endl;
    int maxIter = 100;          // iteration for refine
    // refine for 100 iteration
    gettimeofday(&start, NULL); // start timer
    for (int i = 0; i < maxIter; ++ i) {
        Placer::entire_matching(canvas, texture);
    }
    gettimeofday(&end, NULL);   // stop timer
    // get refine time
    refineTime = (end.tv_sec - start.tv_sec) * 1e6;
    refineTime = (refineTime + (end.tv_sec - start.tv_sec)) * 1e6;
    
    
    std::cout << "Writing result into " << argv[2] << " ..." << std::endl;
    canvas->write(argv[2]);

    // print runtime
    std::cout << "runtime:" << std::endl;
    std::cout << std::setprecision(6)
                << "placer time: " << placerTime << " sec "
                << "refine time: " << refineTime << " sec "
                << "refine per iter avg: " << refineTime / (maxIter * 1.0)
                << " sec " << std::endl;
    
    return 0;
}
