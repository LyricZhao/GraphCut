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

    // Bigger means more randomness
    static constexpr double possibility_k = 0.3;

    static void entire_matching(const std::shared_ptr<Canvas> &canvas, const std::shared_ptr<Image> &texture, bool random=false, int times=100) {
        std::shared_ptr<Patch> best_patch;

        if (random) {
            auto random_x = Random(0, canvas->w - 1);
            auto random_y = Random(0, canvas->h - 1);

            uint64_t best_ssd = UINT64_MAX;
            for (int i = 0; i < times; ++ i) {
                auto patch = std::make_shared<Patch>(texture, random_x(), random_y());
                uint64_t ssd = canvas->ssd(patch);
                if (ssd < best_ssd) {
                    best_ssd = ssd;
                    best_patch = patch;
                }
            }
        } else {
            // FFT-based acceleration
            // Prefix sum
            assert(canvas->none_empty());
            auto *texture_sum = static_cast<uint64_t*> (std::malloc(texture->w * texture->h * sizeof(uint64_t)));
            auto *canvas_sum = static_cast<uint64_t*> (std::malloc(canvas->w * canvas->h * sizeof(uint64_t)));
            auto query = [](const uint64_t *sum, int x, int y, int size_x, int size_y, int w, int h) {
                int last_x = x + size_x - 1, last_y = y + size_y - 1;
                uint64_t result = sum[last_y * w + last_x];
                result += (x > 0 and y > 0) ? sum[(y - 1) * w + x - 1] : 0;
                result -= x > 0 ? sum[last_y * w + x - 1] : 0;
                result -= y > 0 ? sum[(y - 1) * w + last_x] : 0;
                return result;
            };
            auto do_prefix_sum = [](int w, int h, Pixel *pixels, uint64_t *sum) {
                for (int y = 0, index = 0; y < h; ++ y) {
                    for (int x = 0; x < w; ++ x, ++ index) {
                        auto up = y > 0 ? sum[index - w] : 0;
                        auto left = x > 0 ? sum[index - 1] : 0;
                        auto left_up = (y > 0 and x > 0) ? sum[index - w - 1] : 0;
                        sum[index] = up + left + pixels[index].sqr_sum() - left_up;
                    }
                }
            };
            do_prefix_sum(texture->w, texture->h, texture->data, texture_sum);
            do_prefix_sum(canvas->w, canvas->h, canvas->data, canvas_sum);

            // FFT
            auto flipped = texture->flip();
            int dft_w = dft_round(texture->w + canvas->w), dft_h = dft_round(texture->h + canvas->h);
            ComplexPixel *dft_space1, *dft_space2;
            dft_alloc(flipped, dft_w, dft_h, dft_space1);
            dft_alloc(canvas, dft_w, dft_h, dft_space2);
            dft(dft_w, dft_h, dft_space1);
            dft(dft_w, dft_h, dft_space2);
            dft_multiply(dft_w, dft_h, dft_space1, dft_space2);
            dft(dft_w, dft_h, dft_space1, true);

            // Get results
            uint64_t variance = texture->variance();
            auto *possibility = static_cast<double*> (std::malloc(canvas->h * canvas->w * sizeof(double)));
            for (int y = 0, index = 0; y < canvas->h; ++ y) {
                for (int x = 0; x < canvas->w; ++ x, ++ index) {
                    int overlapped_w = std::min(texture->w, canvas->w - x);
                    int overlapped_h = std::min(texture->h, canvas->h - y);
                    uint64_t ssd = 0;
                    ssd += texture_sum[(overlapped_h - 1) * texture->w + overlapped_w - 1];
                    ssd += query(canvas_sum, x, y, overlapped_w, overlapped_h, canvas->w, canvas->h);
                    ssd -= std::floor(2.0 * dft_space1[(texture->h + y - 1) * dft_w + texture->w + x - 1].real_sum());
                    ssd /= overlapped_w * overlapped_h;
                    possibility[index] = std::exp(-1.0 * ssd / (possibility_k * variance));
                }
            }
            double possibility_sum = 0;
            for (int i = 0; i < canvas->h * canvas->w; ++ i) {
                possibility_sum += possibility[i];
            }
            double position = Random<double>(0, 1)(), up = 0;
            for (int y = 0, index = 0; y < canvas->h and not best_patch; ++ y) {
                for (int x = 0; x < canvas->w; ++ x, ++ index) {
                    possibility[index] /= possibility_sum;
                    if (up + possibility[index] >= position) {
                        best_patch = std::make_shared<Patch>(texture, x, y);
                        break;
                    }
                    up += possibility[index];
                }
            }
            assert(best_patch);

            // Free resources
            std::free(possibility);
            std::free(texture_sum);
            std::free(canvas_sum);
            dft_free(dft_space1);
            dft_free(dft_space2);
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