#pragma once

#include <queue>


struct Edge {
    int v, next, capacity;
};


class Graph {
private:
    std::vector<int> depth;

    bool dinic_bfs(int s, int t) {
        std::fill(depth.begin(), depth.end(), 0);
        depth[s] = 1;
        std::queue<int> queue;
        queue.push(s);

        int count = 0;
        while (not queue.empty()) {
            ++ count;
            int u = queue.front();
            queue.pop();
            for (int i = head[u]; i != -1; i = edges[i].next) {
                if (not depth[edges[i].v] and edges[i].capacity) {
                    depth[edges[i].v] = depth[u] + 1;
                    queue.push(edges[i].v);
                }
            }
        }
        return depth[t] > 0;
    }

    int dinic_dfs(int u, int t, int capacity) {
        if (u == t or capacity == 0) {
            return capacity;
        }

        int flow, total_flow = 0;
        for (int i = head[u]; i != -1 and capacity > 0; i = edges[i].next) {
            if (depth[edges[i].v] == depth[u] + 1 and
                (flow = dinic_dfs(edges[i].v, t, std::min(capacity, edges[i].capacity))) > 0) {
                edges[i].capacity -= flow;
                edges[i ^ 1].capacity += flow;
                total_flow += flow;
                capacity -= flow;
            }
        }
        if (not total_flow) {
            depth[u] = 0;
        }
        return total_flow;
    }

    void dinic(int s, int t) {
        while (dinic_bfs(s, t)) {
            while (dinic_dfs(s, t, inf_flow));
        }
    }

    [[nodiscard]] std::vector<bool> bfs_decisions(int s) const {
        std::vector<bool> visited(head.size(), false);
        std::vector<bool> decisions(head.size(), true);
        std::queue<int> queue;
        queue.push(s);
        visited[s] = true;

        while (not queue.empty()) {
            int u = queue.front();
            queue.pop();
            decisions[u] = false;
            for (int i = head[u]; i != -1; i = edges[i].next) {
                if (not visited[edges[i].v] and edges[i].capacity) {
                    visited[edges[i].v] = true;
                    queue.push(edges[i].v);
                }
            }
        }
        return decisions;
    }

public:
    std::vector<int> head;
    std::vector<Edge> edges;

    static constexpr int inf_flow = 1 << 20;

    explicit Graph(int n): head(n, -1), depth(n) {}

    /// Add a bi-directional edge
    void add_edge(int u, int v, int w) {
        assert(0 <= u and u < head.size());
        assert(0 <= v and v < head.size());
        edges.push_back(Edge{v, head[u], w});
        head[u] = edges.size() - 1;
        edges.push_back(Edge{u, head[v], w});
        head[v] = edges.size() - 1;
    }

    [[nodiscard]] std::vector<bool> min_cut(int s, int t) {
        dinic(s, t);
        return bfs_decisions(s);
    }
};
