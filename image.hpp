#pragma once

#include <cmath>
#include <vector>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "graph.hpp"


#pragma pack()
struct Pixel {
    uint8_t r, g, b;

    [[nodiscard]] int distance(const Pixel &pixel) const {
        int r_d = static_cast<int> (r) - pixel.r;
        int g_d = static_cast<int> (g) - pixel.g;
        int b_d = static_cast<int> (b) - pixel.b;
        return std::sqrt(r_d * r_d + g_d * g_d + b_d * b_d);
    }
};

static_assert(sizeof(Pixel) == 3);


class Image {
private:
    bool from_stbi;

public:
    int w = 0, h = 0;
    Pixel *data = nullptr;

    explicit Image(const std::string &path) {
        from_stbi = true;
        int c;
        data = reinterpret_cast<Pixel*> (stbi_load(path.c_str(), &w, &h, &c, 3));
        if (not data) {
            std::cerr << "Unable to load image from " << path << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    Image(int w, int h): w(w), h(h) {
        from_stbi = false;
        data = static_cast<Pixel*> (std::malloc(w * h * sizeof(Pixel)));
    }

    ~Image() {
        if (data) {
            from_stbi ? stbi_image_free(data) : std::free(data);
            data = nullptr;
        }
    }

    void write(const std::string &path) const {
        assert(data);
        if (not stbi_write_png(path.c_str(), w, h, 3, reinterpret_cast<uint8_t*>(data), 0)) {
            std::cerr << "Unable to write image to " << path << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    inline void set(int x, int y, const Pixel &pixel) const {
        assert(0 <= x and x < w);
        assert(0 <= y and y < h);
        data[y * w + x] = pixel;
    }

    [[nodiscard]] inline Pixel pixel(int x, int y) const {
        assert(0 <= x and x < w);
        assert(0 <= y and y < h);
        return data[y * w + x];
    }
};


class Patch {
public:
    int x, y;
    std::shared_ptr<Image> image;

    Patch(const std::shared_ptr<Image> &image, int x, int y): x(x), y(y), image(image) {}

    [[nodiscard]] inline int x_end() const {
        return x + image->w;
    }

    [[nodiscard]] inline int y_end() const {
        return y + image->h;
    }

    [[nodiscard]] inline Pixel pixel(int a, int b) const {
        return image->pixel(a - x, b - y);
    }
};


class Canvas: public Image {
private:
    std::vector<std::shared_ptr<Patch>> origin;

public:
    Canvas(int w, int h): Image(w, h), origin(w * h) {}

    void apply(const std::shared_ptr<Patch> &patch) {
        int x_end = std::min(patch->x_end(), w);
        int y_end = std::min(patch->y_end(), h);

        // Fill non-overlapped area first
        std::vector<std::pair<int, int>> overlapped;
        for (int y = patch->y; y < y_end; ++ y) {
            for (int x = patch->x; x < x_end; ++ x) {
                int index = y * w + x;
                if (not origin[index]) {
                    origin[index] = patch;
                    data[index] = patch->pixel(x, y);
                } else {
                    overlapped.emplace_back(x, y);
                }
            }
        }

        // Build graph
        Graph graph(overlapped.size() + 2);
        int s = overlapped.size(), t = overlapped.size() + 1;
        for (int y = patch->y; y < y_end - 1; ++ y) {
            for (int x = patch->x; x < x_end - 1; ++ x) {
                int index = y * w + x;
                // TODO: Build graph
            }
        }

        // Min-cut and overwrite
        auto decisions = graph.min_cut(s, t);
        assert(decisions.size() == overlapped.size() + 2);
        for (int i = 0; i < decisions.size(); ++ i) {
            if (decisions[i]) { // Belongs to T
                int x = overlapped[i].first, y = overlapped[i].second;
                int index = y * w + x;
                origin[index] = patch;
                data[index] = patch->pixel(x, y);
            }
        }
    }
};