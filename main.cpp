#include "bits/stdc++.h"
using namespace std;

struct Graph {
  int n;
  vector<vector<int>> adj;
  vector<long long> w;

  // Tree DP for MWIS on trees
  // dp[v][0] = max weight IS in subtree of v, not taking v
  // dp[v][1] = max weight IS in subtree of v, taking v
  pair<long long, vector<int>> solveTree(int root) {
    int sz = n;
    vector<long long> dp0(sz, 0), dp1(sz, 0);
    vector<int> parent(sz, -1);
    vector<int> order;

    // BFS to get processing order
    vector<bool> vis(sz, false);
    queue<int> q;
    q.push(root);
    vis[root] = true;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      order.push_back(u);
      for (int v : adj[u]) {
        if (!vis[v]) {
          vis[v] = true;
          parent[v] = u;
          q.push(v);
        }
      }
    }

    // Process in reverse BFS order (leaves first)
    for (int i = (int)order.size() - 1; i >= 0; i--) {
      int u = order[i];
      dp1[u] = w[u];
      dp0[u] = 0;
      for (int v : adj[u]) {
        if (v == parent[u])
          continue;
        dp1[u] += dp0[v];
        dp0[u] += max(dp0[v], dp1[v]);
      }
    }

    // Backtrack to find the actual set
    vector<int> selected;
    vector<int> take(sz, -1);
    // Decide root
    if (dp1[root] >= dp0[root]) {
      take[root] = 1;
    } else {
      take[root] = 0;
    }

    for (int u : order) {
      if (take[u] == 1) {
        selected.push_back(u);
        for (int v : adj[u]) {
          if (v == parent[u])
            continue;
          take[v] = 0;
        }
      } else {
        for (int v : adj[u]) {
          if (v == parent[u])
            continue;
          if (dp1[v] >= dp0[v]) {
            take[v] = 1;
          } else {
            take[v] = 0;
          }
        }
      }
    }

    long long total = max(dp0[root], dp1[root]);
    return {total, selected};
  }

  // Check if bipartite and return coloring
  pair<bool, vector<int>> checkBipartite() {
    vector<int> color(n, -1);
    bool bip = true;
    for (int i = 0; i < n && bip; i++) {
      if (color[i] != -1)
        continue;
      queue<int> q;
      q.push(i);
      color[i] = 0;
      while (!q.empty() && bip) {
        int u = q.front();
        q.pop();
        for (int v : adj[u]) {
          if (color[v] == -1) {
            color[v] = 1 - color[u];
            q.push(v);
          } else if (color[v] == color[u]) {
            bip = false;
          }
        }
      }
    }
    return {bip, color};
  }

  // Bitmask DP for small components
  pair<long long, vector<int>> solveBitmask() {
    // Build adjacency bitmask
    vector<int> nbr(n, 0);
    for (int u = 0; u < n; u++) {
      for (int v : adj[u]) {
        nbr[u] |= (1 << v);
      }
    }

    int full = (1 << n);
    vector<long long> dp(full, -1);
    dp[0] = 0;
    long long best = 0;
    int best_mask = 0;

    for (int mask = 0; mask < full; mask++) {
      if (dp[mask] == -1)
        continue;
      if (dp[mask] > best) {
        best = dp[mask];
        best_mask = mask;
      }
      // Find next candidate to add
      // The lowest unset bit that's not a neighbor of any set bit
      int blocked = 0;
      for (int i = 0; i < n; i++) {
        if (mask & (1 << i))
          blocked |= nbr[i];
      }
      blocked |= mask; // already selected

      for (int i = 0; i < n; i++) {
        if (blocked & (1 << i))
          continue;
        int new_mask = mask | (1 << i);
        if (dp[new_mask] < dp[mask] + w[i]) {
          dp[new_mask] = dp[mask] + w[i];
        }
      }
    }

    vector<int> selected;
    for (int i = 0; i < n; i++) {
      if (best_mask & (1 << i))
        selected.push_back(i);
    }
    return {best, selected};
  }

  // Dinic's max flow
  struct Edge {
    int to, rev;
    long long cap;
  };

  struct Dinic {
    vector<vector<Edge>> graph;
    vector<int> level, iter;
    int n;

    Dinic(int n) : n(n), graph(n), level(n), iter(n) {}

    void addEdge(int from, int to, long long cap) {
      graph[from].push_back({to, (int)graph[to].size(), cap});
      graph[to].push_back({from, (int)graph[from].size() - 1, 0});
    }

    bool bfs(int s, int t) {
      fill(level.begin(), level.end(), -1);
      queue<int> q;
      level[s] = 0;
      q.push(s);
      while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (auto &e : graph[v]) {
          if (e.cap > 0 && level[e.to] < 0) {
            level[e.to] = level[v] + 1;
            q.push(e.to);
          }
        }
      }
      return level[t] >= 0;
    }

    long long dfs(int v, int t, long long f) {
      if (v == t)
        return f;
      for (int &i = iter[v]; i < (int)graph[v].size(); i++) {
        Edge &e = graph[v][i];
        if (e.cap > 0 && level[v] < level[e.to]) {
          long long d = dfs(e.to, t, min(f, e.cap));
          if (d > 0) {
            e.cap -= d;
            graph[e.to][e.rev].cap += d;
            return d;
          }
        }
      }
      return 0;
    }

    long long maxflow(int s, int t) {
      long long flow = 0;
      while (bfs(s, t)) {
        fill(iter.begin(), iter.end(), 0);
        long long d;
        while ((d = dfs(s, t, LLONG_MAX)) > 0)
          flow += d;
      }
      return flow;
    }

    // After computing max flow, find min cut reachable set from source
    vector<bool> minCutReachable(int s) {
      vector<bool> reach(n, false);
      queue<int> q;
      q.push(s);
      reach[s] = true;
      while (!q.empty()) {
        int v = q.front();
        q.pop();
        for (auto &e : graph[v]) {
          if (e.cap > 0 && !reach[e.to]) {
            reach[e.to] = true;
            q.push(e.to);
          }
        }
      }
      return reach;
    }
  };

  pair<long long, vector<int>> solveBipartite(const vector<int> &color) {
    // Left = color 0, Right = color 1
    // source = n, sink = n+1
    int S = n, T = n + 1;
    Dinic dinic(n + 2);

    long long total_weight = 0;
    for (int i = 0; i < n; i++)
      total_weight += w[i];

    for (int i = 0; i < n; i++) {
      if (color[i] == 0) {
        dinic.addEdge(S, i, w[i]);
        for (int j : adj[i]) {
          dinic.addEdge(i, j, LLONG_MAX / 2);
        }
      } else {
        dinic.addEdge(i, T, w[i]);
      }
    }

    long long min_vc = dinic.maxflow(S, T);
    long long mwis = total_weight - min_vc;

    // Find the IS: nodes NOT in the min vertex cover
    // Min vertex cover from König's theorem on residual graph:
    // In the residual, find reachable from S
    // VC = left NOT reachable + right reachable
    // IS = complement = left reachable + right NOT reachable
    auto reach = dinic.minCutReachable(S);

    vector<int> selected;
    for (int i = 0; i < n; i++) {
      if (color[i] == 0 && reach[i])
        selected.push_back(i);
      if (color[i] == 1 && !reach[i])
        selected.push_back(i);
    }

    return {mwis, selected};
  }

  // Greedy + local search for general graphs
  pair<long long, vector<int>> solveGreedy() {
    // Greedy: priority = weight / (degree + 1), pick highest, remove neighbors
    vector<int> deg(n);
    for (int i = 0; i < n; i++)
      deg[i] = adj[i].size();

    // Use the ratio-based greedy
    vector<bool> removed(n, false);
    vector<bool> inIS(n, false);

    // Compute effective weight = w[i], effective degree
    // Priority: w[i] / (deg[i] + 1)
    auto cmp = [&](int a, int b) {
      return (double)w[a] / (deg[a] + 1) < (double)w[b] / (deg[b] + 1);
    };
    priority_queue<int, vector<int>, decltype(cmp)> pq(cmp);
    for (int i = 0; i < n; i++)
      pq.push(i);

    while (!pq.empty()) {
      int u = pq.top();
      pq.pop();
      if (removed[u])
        continue;
      inIS[u] = true;
      removed[u] = true;
      for (int v : adj[u]) {
        if (!removed[v]) {
          removed[v] = true;
          // Update degrees of v's neighbors
          for (int w2 : adj[v]) {
            if (!removed[w2]) {
              deg[w2]--;
              pq.push(w2); // re-insert with updated priority
            }
          }
        }
      }
    }

    // Local search: try swapping one node out for its neighbors in
    bool improved = true;
    int iterations = 0;
    while (improved && iterations < 1000) {
      improved = false;
      iterations++;
      for (int u = 0; u < n; u++) {
        if (!inIS[u])
          continue;
        // Try removing u and adding some of its neighbors
        // Collect neighbors of u that are NOT in IS
        vector<int> candidates;
        for (int v : adj[u]) {
          if (!inIS[v]) {
            // Check if v's only IS-neighbor is u
            bool can_add = true;
            for (int w2 : adj[v]) {
              if (w2 != u && inIS[w2]) {
                can_add = false;
                break;
              }
            }
            if (can_add)
              candidates.push_back(v);
          }
        }

        long long gain = 0;
        for (int v : candidates)
          gain += w[v];

        if (gain > w[u]) {
          // Swap: remove u, add candidates
          inIS[u] = false;
          for (int v : candidates)
            inIS[v] = true;
          improved = true;
        }
      }
    }

    // Additional: try 2-swap (remove 2, add their freed neighbors)
    // This is expensive but can help
    improved = true;
    iterations = 0;
    while (improved && iterations < 100) {
      improved = false;
      iterations++;

      for (int u = 0; u < n && !improved; u++) {
        if (!inIS[u])
          continue;
        for (int v : adj[u]) {
          if (!inIS[v])
            continue; // should not happen in IS
        }
        // Try removing u
        // For each non-IS neighbor v of u:
        //   what if we add v? Then v's IS-neighbors (which is just u if we
        //   removed u) need to be checked
        // More sophisticated: remove u, see what nodes become free

        // Freed nodes if u is removed: non-IS nodes whose only IS neighbor was
        // u
        vector<int> freed;
        for (int v : adj[u]) {
          if (inIS[v])
            continue;
          bool only_u = true;
          for (int w2 : adj[v]) {
            if (w2 != u && inIS[w2]) {
              only_u = false;
              break;
            }
          }
          if (only_u)
            freed.push_back(v);
        }

        // Greedy pick among freed nodes (they might conflict with each other)
        // Build subgraph of freed and pick greedy IS
        set<int> freed_set(freed.begin(), freed.end());
        vector<bool> freed_removed(n, false);
        long long freed_gain = 0;
        vector<int> freed_picked;

        // Sort freed by weight descending
        sort(freed.begin(), freed.end(),
             [&](int a, int b) { return w[a] > w[b]; });

        for (int f : freed) {
          if (freed_removed[f])
            continue;
          freed_picked.push_back(f);
          freed_gain += w[f];
          freed_removed[f] = true;
          for (int nb : adj[f]) {
            if (freed_set.count(nb))
              freed_removed[nb] = true;
          }
        }

        if (freed_gain > w[u]) {
          inIS[u] = false;
          for (int f : freed_picked)
            inIS[f] = true;
          improved = true;
          break;
        }
      }
    }

    vector<int> selected;
    long long total = 0;
    for (int i = 0; i < n; i++) {
      if (inIS[i]) {
        selected.push_back(i);
        total += w[i];
      }
    }
    return {total, selected};
  }
};

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(NULL);

  int n, m;
  cin >> n >> m;

  vector<long long> rating(n + 1);
  for (int i = 1; i <= n; i++)
    cin >> rating[i];

  vector<vector<int>> adj(n + 1);
  for (int i = 0; i < m; i++) {
    int u, v;
    cin >> u >> v;
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  // Find connected components
  vector<bool> vis(n + 1, false);
  long long total_ans = 0;
  vector<int> selected_nodes;

  for (int i = 1; i <= n; i++) {
    if (vis[i])
      continue;

    // BFS to find component
    queue<int> q;
    q.push(i);
    vis[i] = true;
    vector<int> comp;
    int edge_count = 0;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      comp.push_back(u);
      edge_count += adj[u].size();
      for (int v : adj[u]) {
        if (!vis[v]) {
          vis[v] = true;
          q.push(v);
        }
      }
    }
    edge_count /= 2;
    int sz = comp.size();

    // Single node
    if (sz == 1) {
      total_ans += rating[comp[0]];
      selected_nodes.push_back(comp[0]);
      continue;
    }

    // Build local graph
    Graph g;
    g.n = sz;
    g.adj.resize(sz);
    g.w.resize(sz);

    // Map global -> local indices
    map<int, int> gToL;
    for (int j = 0; j < sz; j++) {
      gToL[comp[j]] = j;
      g.w[j] = rating[comp[j]];
    }
    for (int j = 0; j < sz; j++) {
      int u = comp[j];
      for (int v : adj[u]) {
        if (gToL.count(v)) {
          g.adj[j].push_back(gToL[v]);
        }
      }
    }

    pair<long long, vector<int>> result;

    // Check if tree
    if (edge_count == sz - 1) {
      result = g.solveTree(0);
    }
    // Check if bipartite
    else {
      auto [bip, color] = g.checkBipartite();
      if (bip) {
        result = g.solveBipartite(color);
      }
      // Small enough for bitmask
      else if (sz <= 24) {
        result = g.solveBitmask();
      }
      // General: greedy + local search
      else {
        result = g.solveGreedy();
      }
    }

    total_ans += result.first;
    for (int j : result.second) {
      selected_nodes.push_back(comp[j]);
    }
  }

  sort(selected_nodes.begin(), selected_nodes.end());

  cout << total_ans << "\n";
  for (int i = 0; i < (int)selected_nodes.size(); i++) {
    if (i)
      cout << " ";
    cout << selected_nodes[i];
  }
  cout << "\n";

  return 0;
}