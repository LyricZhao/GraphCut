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

    [[nodiscard]] inline bool in_range(int a, int b) const {
        return 0 <= a - x and a - x < image->w and 0 <= b - y and b - y < image->h;
    }
};


class Canvas: public Image {
private:
    std::vector<std::shared_ptr<Patch>> origin;

public:
    Canvas(int w, int h): Image(w, h), origin(w * h) {}

    [[nodiscard]] inline bool in_range(int x, int y) const {
        return 0 <= x and x < w and 0 <= y and y < h;
    }

    void apply(const std::shared_ptr<Patch> &patch) {
        std::cout << " > Applying a new patch at (" << patch->x << ", " << patch->y << ")" << std::endl;
        int x_end = std::min(patch->x_end(), w);
        int y_end = std::min(patch->y_end(), h);

        // Fill non-overlapped area first
        static int dx[4] = { 0, +1,  0, -1};
        static int dy[4] = {+1,  0, -1,  0};
        int n_old_seam_nodes = 0;
        std::vector<std::pair<int, int>> overlapped;
        std::vector<int> overlapped_index(w * h, -1);
        for (int y = patch->y; y < y_end; ++ y) {
            for (int x = patch->x; x < x_end; ++ x) {
                int index = y * w + x;
                if (not origin[index]) {
                    origin[index] = patch;
                    data[index] = patch->pixel(x, y);
                } else {
                    assert(origin[index] != patch);
                    overlapped_index[index] = overlapped.size();
                    overlapped.emplace_back(x, y);
                    for (int d = 0; d < 2; ++ d) {
                        int a = x + dx[d], b = y + dy[d];
                        int neighbor_index = b * w + a;
                        if (in_range(a, b) and origin[neighbor_index] and origin[neighbor_index] != origin[index]) {
                            ++ n_old_seam_nodes;
                        }
                    }
                }
            }
        }

        // Build graph
        Graph graph(overlapped.size() + n_old_seam_nodes + 2);
        int s = overlapped.size() + n_old_seam_nodes, t = overlapped.size() + n_old_seam_nodes + 1;
        int old_sean_node_index = overlapped.size();
        for (int i = 0; i < overlapped.size(); ++ i) {
            auto [x, y] = overlapped[i];
            int index = y * w + x;
            int m_s = pixel(x, y).distance(patch->pixel(x, y));
            for (int d = 0; d < 4; ++ d) {
                int a = x + dx[d], b = y + dy[d];
                int neighbor_index = b * w + a;
                if (in_range(a, b) and origin[neighbor_index]) {
                    if (origin[neighbor_index] == patch) {
                        graph.add_edge(i, t, Graph::inf_flow);
                    } else {
                        if (overlapped_index[neighbor_index] == -1) {
                            graph.add_edge(s, i, Graph::inf_flow);
                        } else if (d < 2) { // `add_edge` is bi-directional
                            if (origin[index] != origin[neighbor_index] and origin[index]->in_range(a, b) and
                                origin[neighbor_index]->in_range(a, b)) { // Old seam node
                                int m_t = data[neighbor_index].distance(patch->pixel(a, b));
                                graph.add_edge(old_sean_node_index, i, m_s + m_t);
                                graph.add_edge(old_sean_node_index, overlapped_index[neighbor_index], m_s + m_t);
                                int old_m_s = origin[index]->pixel(x, y).distance(origin[neighbor_index]->pixel(x, y));
                                int old_m_t = origin[index]->pixel(a, b).distance(origin[neighbor_index]->pixel(a, b));
                                graph.add_edge(old_sean_node_index, t, old_m_s + old_m_t);
                                ++ old_sean_node_index;
                            } else {
                                int m_t = data[neighbor_index].distance(patch->pixel(a, b));
                                graph.add_edge(i, overlapped_index[neighbor_index], m_s + m_t);
                            }
                        }
                    }
                }
            }
        }
        // std::cout << " > " << overlapped.size() << " overlapped pixels" << std::endl;

        // Min-cut and overwrite
        // std::cout << " > Running min-cut algorithm ... " << std::endl;
        auto decisions = graph.min_cut(s, t);
        assert(decisions.size() == overlapped.size() + n_old_seam_nodes + 2);
        for (int i = 0; i < overlapped.size(); ++ i) {
            if (decisions[i]) { // Belongs to the new patch
                auto [x, y] = overlapped[i];
                int index = y * w + x;
                origin[index] = patch;
                data[index] = patch->pixel(x, y);
            }
        }
    }
};