# CSE 318 — Artificial Intelligence Sessional

This repository contains the solutions to all offline assignments of the course **CSE 318: Artificial Intelligence Sessional**.

**Student ID:** 2105052

---

## Assignments Overview

| # | Assignment | Topic | Language | Key Algorithm |
|---|-----------|-------|----------|---------------|
| 1 | [N-Puzzle](#1-n-puzzle) | Informed Search | Python | A* Search with multiple heuristics |
| 2 | [Adversarial Search](#2-adversarial-search--chain-reaction-game) | Adversarial Game Search | Python | Minimax with Alpha-Beta Pruning |
| 3 | [Decision Tree](#3-decision-tree) | Supervised Learning / Classification | C++ | Decision Tree with 3 splitting criteria |
| 4 | [Max-Cut](#4-max-cut-problem) | Combinatorial Optimization | Python | GRASP + Local Search |

---

## 1. N-Puzzle

**Directory:** [`Npuzzle/`](Npuzzle/)

### Problem Description

The classic N-Puzzle (sliding tile puzzle) where numbered tiles on an $n \times n$ grid must be rearranged from a scrambled configuration to the goal state by sliding tiles into the blank space.

### Algorithms & Techniques

- **A\* Search** with priority queue for optimal pathfinding
- **Solvability check** using inversion count before attempting to solve
- Four heuristic functions compared:
  | Heuristic | Description |
  |-----------|-------------|
  | **Hamming Distance** | Count of misplaced tiles |
  | **Manhattan Distance** | Sum of taxicab distances of each tile from its goal position |
  | **Euclidean Distance** | Sum of straight-line distances of each tile from its goal |
  | **Linear Conflict** | Manhattan Distance + penalty for tiles in correct row/column but in wrong order |

### Input / Output

- **Input:** Puzzle size $n$, followed by the $n \times n$ grid (0 represents blank)
- **Output:** Step-by-step solution (each board state), minimum moves, nodes expanded, and nodes explored

### Key Files

- [`2105052/2105052_npuzzle.py`](Npuzzle/2105052/2105052_npuzzle.py) — Main solver
- [`2105052/package.py`](Npuzzle/2105052/package.py) — Board class and heuristic implementations

---

## 2. Adversarial Search — Chain Reaction Game

**Directory:** [`adv_search/`](adv_search/)

### Problem Description

A two-player **Chain Reaction** game on a $9 \times 6$ grid. Players alternately place orbs in cells. When the number of orbs in a cell reaches its critical mass (determined by position — corner, edge, or interior), the cell explodes, distributing orbs to adjacent cells and capturing opponent pieces. The game ends when only one player's orbs remain.

### Algorithms & Techniques

- **Minimax** with **Alpha-Beta Pruning** for game tree search
- Configurable search depth and time limits
- Five heuristic evaluation functions:
  | # | Heuristic | Strategy |
  |---|-----------|----------|
  | 1 | Orb Count Difference | Material advantage |
  | 2 | Close to Explosion | Proximity to critical mass |
  | 3 | Corner/Edge Advantage | Positional dominance |
  | 4 | Ready to Explode | Offensive threat potential |
  | 5 | Hybrid | Weighted combination of all above |

### Game Modes

- Human vs AI
- AI vs AI
- Random vs AI

### Key Files

- [`backend.py`](adv_search/backend.py) — Game logic, minimax, heuristics
- [`frontend.py`](adv_search/frontend.py) — Pygame-based interactive UI
- [`back2.py`](adv_search/back2.py) — Experimental analysis (depth/heuristic comparison)

---

## 3. Decision Tree

**Directory:** [`decision/`](decision/)

### Problem Description

Build and evaluate decision trees for classification using three different splitting criteria across multiple datasets. Analyze trade-offs between accuracy, tree complexity, and training time.

### Algorithms & Techniques

- **Decision Tree Construction** with recursive splitting
- Handles both numeric (midpoint thresholding) and categorical attributes
- Three splitting criteria compared:
  | Criterion | Description |
  |-----------|-------------|
  | **Information Gain (IG)** | Standard entropy-based splitting |
  | **Information Gain Ratio (IGR)** | IG normalized by intrinsic value to handle high-branching attributes |
  | **Normalized Weighted IG (NWIG)** | IG normalized by $\log_2(k)$ to penalize excessive splits |
- **Pruning** via max depth limits (0, 3, 5, 10, 15)
- 10 trials per configuration for statistical robustness

### Datasets

- Iris dataset
- Adult dataset

### Evaluation Metrics

- Classification accuracy (mean ± std over 10 runs)
- Tree complexity (node count, actual depth)
- Training time

### Key Files

- [`submission/2105052.cpp`](decision/submission/2105052.cpp) — Decision tree implementation
- [`generate_plots.py`](decision/generate_plots.py) — Automated visualization generation
- [`report.md`](decision/report.md) — Detailed analysis report

---

## 4. Max-Cut Problem

**Directory:** [`max_cut/`](max_cut/)

### Problem Description

The **Maximum Cut** problem: partition the vertices of a weighted undirected graph into two disjoint sets $X$ and $Y$ to maximize the total weight of edges crossing between the two sets.

### Algorithms & Techniques

| Algorithm | Description |
|-----------|-------------|
| **Simple Randomized** | Random vertex partitioning, repeated $n$ times |
| **Simple Greedy** | Start with heaviest edge, greedily assign remaining vertices |
| **Semi-Greedy** | GRASP construction using Restricted Candidate List (RCL) with tunable $\alpha$ |
| **Local Search** | Iteratively move vertices between sets while improvement exists |
| **GRASP** | Greedy Randomized Adaptive Search Procedure — semi-greedy construction + local search improvement, multiple restarts with varying $\alpha$ values (0.5, 0.75, 0.9, 0.95, 0.99) |

### Benchmark

- Tested on **54 standard benchmark graphs** (set1/g1–g54)
- Results compared against known best solutions

### Key Files

- [`2105052.py`](max_cut/2105052.py) — All algorithms implementation
- [`2105052.csv`](max_cut/2105052.csv) — Results across all graphs and algorithms
- [`2105052_plot.py`](max_cut/2105052_plot.py) — Visualization of results
- [`set1/`](max_cut/set1/) — Benchmark graph files (.rud format)

---

## How to Run

### N-Puzzle
```bash
cd Npuzzle/2105052
python 2105052_npuzzle.py < input.txt
```

### Chain Reaction (Adversarial Search)
```bash
cd adv_search
python frontend.py
```

### Decision Tree
```bash
cd decision/submission
g++ -o 2105052 2105052.cpp
./2105052
```

### Max-Cut
```bash
cd max_cut
python 2105052.py
```

---

## Course Information

- **Course:** CSE 318 — Artificial Intelligence
- **Type:** Sessional (Lab)
- **Department:** CSE, BUET
