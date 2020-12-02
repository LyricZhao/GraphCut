#pragma once

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"


#pragma pack()
struct Pixel {
    uint8_t r, g, b;
};

static_assert(sizeof(Pixel) == 3);


class Image {
private:
    bool from_stbi;
    Pixel *data = nullptr;

public:
    int w = 0, h = 0;

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

    void write(const std::string &path) {
        assert(data);
        if (not stbi_write_png(path.c_str(), w, h, 3, reinterpret_cast<uint8_t*>(data), 0)) {
            std::cerr << "Unable to write image to " << path << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    inline void set(int x, int y, const Pixel &pixel) {
        assert(0 <= x and x < w);
        assert(0 <= y and y < h);
        data[y * w + x] = pixel;
    }

    inline Pixel pixel(int x, int y) {
        assert(0 <= x and x < w);
        assert(0 <= y and y < h);
        return data[y * w + x];
    }
};