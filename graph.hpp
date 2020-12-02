#pragma once

#include <queue>


struct Edge {
    int v, next;
    int capacity, flow;
};


class Graph {
private:
    std::vector<int> depth;

    bool dinic_bfs(int s, int t) {
        depth.resize(head.size(), 0);
        depth[s] = 1;
        std::queue<int> queue;
        queue.push(s);

        while (not queue.empty()) {
            int u = queue.front();
            queue.pop();
            for (int i = head[u]; i != -1; i = edges[i].next) {
                if (not depth[edges[i].v] and edges[i].capacity > edges[i].flow) {
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

        int total_flow = 0;
        for (int i = head[u]; i != -1 and capacity > 0; i = edges[i].next) {
            if (depth[edges[i].v] == depth[u] + 1) {
                int flow = dinic_dfs(edges[i].v, t, std::min(capacity, edges[i].capacity - edges[i].flow));
                edges[i].flow += flow;
                edges[i ^ 1].flow -= flow;
                total_flow += flow;
                capacity -= flow;
            }
        }
        return total_flow;
    }

    void dinic(int s, int t) {
        while (dinic_bfs(s, t)) {
            while (dinic_dfs(s, t, INT32_MAX));
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
                if (not visited[edges[i].v] and edges[i].capacity > edges[i].flow) {
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

    explicit Graph(int n): head(n) {}

    void add_edge(int u, int v, int w) {
        if (u >= head.size() or v > head.size()) {
            head.resize(std::max(u, v), -1);
        }
        edges.push_back(Edge{v, head[u], w, 0});
        head[u] = edges.size() - 1;
        edges.push_back(Edge{u, head[v], 0, 0});
        head[v] = edges.size() - 1;
    }

    [[nodiscard]] std::vector<bool> min_cut(int s, int t) {
        dinic(s, t);
        return bfs_decisions(s);
    }
};
