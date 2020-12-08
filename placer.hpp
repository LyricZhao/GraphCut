#pragma once

#include "cherry.hpp"
#include "dft.hpp"
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

    static void random(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture) {
        auto random_x = Random(0, texture->w - 1);
        auto random_y = Random(0, texture->h - 1);
        auto patch = std::make_shared<Patch>(texture, random_x(), random_y());
        canvas->apply(patch);
    }

    static void entire_matching(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture, int times=100) {
        // TODO: FFT-based acceleration
        auto random_x = Random(0, texture->w - 1);
        auto random_y = Random(0, texture->h - 1);

        uint64_t best_ssd = UINT64_MAX;
        std::shared_ptr<Patch> best_patch;
        for (int i = 0; i < times; ++ i) {
            auto patch = std::make_shared<Patch>(texture, random_x(), random_y());
            uint64_t ssd = canvas->ssd(patch);
            if (ssd < best_ssd) {
                best_ssd = ssd;
                best_patch = patch;
            }
        }
        canvas->apply(best_patch);
    }

    static void sub_patch_matching(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture, int times=100) {
        int sub_patch_w = texture->w / 3, sub_patch_h = texture->h / 3;
        auto random_canvas_x = Random(0, canvas->w - sub_patch_w), random_canvas_y = Random(0, canvas->h - sub_patch_h);
        int canvas_x = random_canvas_x(), canvas_y = random_canvas_y();

        auto random_x = Random(0, texture->w - sub_patch_w);
        auto random_y = Random(0, texture->h - sub_patch_h);
        uint64_t best_ssd = UINT64_MAX;
        std::shared_ptr<Patch> best_patch;
        for (int i = 0; i < times; ++ i) {
            int x = random_x(), y = random_y();
            auto patch = std::make_shared<Patch>(texture, canvas_x - x, canvas_y - y);
            uint64_t ssd = canvas->ssd(patch, canvas_x, canvas_y, sub_patch_w, sub_patch_h); // SSD only calculates the sub-patch region
            if (ssd < best_ssd) {
                best_ssd = ssd;
                best_patch = patch;
            }
        }
        canvas->apply(best_patch);
    }
};