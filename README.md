# Hackathon Squad

**Maximum Weight Independent Set (MWIS)** solver for the Hackathon Squad team-selection problem.

Given a pool of coders with skill ratings and conflict edges (pairs who refuse to work together), select a subset that **maximizes total skill rating** with no two selected coders in conflict — i.e. a Maximum Weight Independent Set on a general graph.

## Problem

- **Input:** `n` coders, `m` conflict pairs, skill rating per coder.
- **Output:** Total skill rating of the selected team and the sorted list of selected coder IDs.
- **Constraint:** No two selected coders share a conflict edge (independent set).

## Approach

MWIS is NP-hard on general graphs. The solution decomposes the graph into connected components and applies the **strongest tractable algorithm** for each:

| Graph Structure | Algorithm                                   | Optimality  | Complexity |
| --------------- | ------------------------------------------- | ----------- | ---------- |
| Isolated node   | Direct selection                            | Optimal     | O(1)       |
| Tree (m = n−1)  | Tree DP (take/skip)                         | **Optimal** | O(n)       |
| Bipartite       | Dinic's max-flow + König's theorem          | **Optimal** | O(V²E)     |
| Small (n ≤ 24)  | Bitmask DP                                  | **Optimal** | O(2ⁿ)      |
| General         | Greedy (weight/degree ratio) + local search | Heuristic   | O(n·m)     |

### Key Algorithms

- **Tree DP:** Classic bottom-up DP. For each vertex, compute the best IS weight including vs. excluding it, then backtrack.
- **Bipartite MWIS:** Reduced to minimum weight vertex cover via König's theorem. Min vertex cover = min-cut in a flow network, solved with Dinic's max-flow in O(V²E). The independent set is the complement.
- **Bitmask DP:** Exact enumeration of all 2ⁿ subsets for small components, tracking the best valid IS.
- **Greedy + Local Search:** Initial solution built by greedily selecting nodes with the highest weight-to-degree ratio, then iteratively improved via 1-swap local search (remove a node, add all freed neighbors if beneficial).

## Build & Run

```bash
g++ -O2 -std=c++17 -I. main.cpp -o hackathon_squad
./hackathon_squad < test_suite/input_01_tiny_random.txt
```

## Test Suite

23 test cases covering diverse graph topologies:

| Category         | Tests                                                        |
| ---------------- | ------------------------------------------------------------ |
| Paths, Cycles    | `03_path`, `05_cycle`, `13_all_skills_one`, `14_alternating` |
| Trees            | `06_tree`                                                    |
| Stars            | `04_star`                                                    |
| Complete / Dense | `07_complete`, `15_small_dense`                              |
| Bipartite        | `08_bipartite`, `10_grid`, `11_matching`                     |
| Disjoint Cliques | `09_disjoint_cliques`                                        |
| No Edges         | `12_no_edges`                                                |
| Large Sparse     | `16` (20K), `17` (100K), `18` (200K nodes)                   |
| Legacy           | `legacy_1` through `legacy_5`                                |

Run all tests:

```bash
chmod +x run_tests.sh
./run_tests.sh
```

## Results

All 23 tests produce valid independent sets. The solution **matches or outperforms** the reference on every test case, achieving provably optimal results on trees, bipartite graphs, and small components.

```
Total: 23  |  Match: 15  |  Better: 8  |  Worse: 0
```

## Tech

- **Language:** C++17
- **No external dependencies** — uses only standard library (`bits/stdc++.h`)
- **Performance:** Largest test (200K nodes, 200K edges) solves in < 1 second
