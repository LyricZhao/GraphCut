#pragma once

#include "cherry.hpp"
#include "image.hpp"


class Placer {
public:
    static void init(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture) {
        auto random_y = Random(texture->h / 3, texture->h * 2 / 3);
        auto random_x = Random(texture->w / 3, texture->w * 2 / 3);
        for (int y = 0; y < canvas->h; y += random_y()) {
            for (int x = 0; x < canvas->w; x += random_x()) {
                auto patch = std::make_shared<Patch>(texture, x, y);
                canvas->apply(patch);
            }
        }
    }

    static void random(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture, int times=100) {
        auto random_x = Random(0, texture->w - 1);
        auto random_y = Random(0, texture->h - 1);
        for (int i = 0; i < times; ++ i) {
            auto patch = std::make_shared<Patch>(texture, random_x(), random_y());
            canvas->apply(patch);
        }
    }

    static void entire_matching(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture) {
        // TODO: coding with FFT
    }
};