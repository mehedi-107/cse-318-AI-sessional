import numpy as np
import random
import csv

# --- Graph loading and preprocessing ---
def load_graph(filename):
    with open(filename, 'r') as f:
        n, m = map(int, f.readline().split())
        edges = np.zeros((m, 3), dtype=int)
        for i in range(m):
            u, v, w = map(int, f.readline().split())
            edges[i] = (u-1, v-1, w)
    # build adjacency matrix
    W = np.zeros((n, n), dtype=float)
    for u, v, w in edges:
        W[u, v] = w
        W[v, u] = w
    return n, W

# --- Cut weight via matrix operations ---
def cut_weight(W, mask):
    # mask: boolean array of length n for partition X; Y = ~mask
    X = mask
    Y = ~mask
    # Extract cross edges
    sub = W[np.ix_(X, Y)]
    return sub.sum()

# --- Randomized heuristic ---
def randomized_heuristic(n, W, trials):
    best_mask = None
    best_weight = -np.inf
    for _ in range(trials):
        mask = np.random.rand(n) >= 0.5
        cw = cut_weight(W, mask)
        if cw > best_weight:
            best_weight = cw
            best_mask = mask.copy()
    return best_mask, best_weight

# --- Greedy heuristic ---
def greedy_max_cut(n, W):
    # seed with heaviest edge
    idx = np.unravel_index(np.argmax(W, axis=None), W.shape)
    mask = np.zeros(n, dtype=bool)
    mask[idx[0]] = True
    mask[idx[1]] = False
    assigned = set(idx)
    # assign remaining
    for v in range(n):
        if v in assigned: continue
        # compute contributions
        wx = W[v, ~mask].sum()  # connect to Y
        wy = W[v, mask].sum()   # connect to X
        mask[v] = wx > wy
    return mask, cut_weight(W, mask)

# --- Semi-greedy (value-based RCL) heuristic ---
def semi_greedy_max_cut(n, W, alpha):
    mask = np.zeros(n, dtype=bool)
    unassigned = set(range(n))
    # loop until all assigned
    while unassigned:
        g = np.zeros(n)
        # compute g(v)
        for v in unassigned:
            wx = W[v, ~mask].sum()
            wy = W[v, mask].sum()
            g[v] = max(wx, wy)
        vals = g[list(unassigned)]
        wmin, wmax = vals.min(), vals.max()
        mu = wmin + alpha * (wmax - wmin)
        # RCL
        RCL = [v for v in unassigned if g[v] >= mu]
        v = random.choice(RCL)
        # assign v
        wx = W[v, ~mask].sum()
        wy = W[v, mask].sum()
        mask[v] = wx > wy
        unassigned.remove(v)
    return mask, cut_weight(W, mask)

# --- Optimized local search with incremental gains ---
def local_search(n, W, mask):
    # initial delta computation
    total_w = W.sum(axis=1)
    internal = W.dot(mask.astype(float))
    external = total_w - internal
    # delta[v] = internal[v] - external[v] if v in X else external[v] - internal[v]
    delta = np.where(mask, internal - external, external - internal)

    improved = True
    while improved:
        v = np.argmax(delta)
        if delta[v] <= 0:
            break
        # flip v
        mask[v] = ~mask[v]
        # update internal/external only for neighbors
        nbrs = np.nonzero(W[v] > 0)[0]
        for u in nbrs:
            wvu = W[v, u]
            # if u and v now same side, adjust internal/external
            if mask[u]:
                internal[u] += wvu
                external[u] -= wvu
            else:
                internal[u] -= wvu
                external[u] += wvu
        # update internal/external of v
        internal[v], external[v] = external[v], internal[v]
        # recompute delta for v and its neighbors
        to_update = np.append(nbrs, v)
        for u in to_update:
            delta[u] = (internal[u] - external[u]) if mask[u] else (external[u] - internal[u])
    return mask, cut_weight(W, mask)

# --- GRASP metaheuristic ---
def grasp(n, W, max_iter=100, alpha=0.5):
    best_mask = None
    best_weight = -np.inf
    for _ in range(max_iter):
        mask, _ = semi_greedy_max_cut(n, W, alpha)
        mask, _ = local_search(n, W, mask)
        cw = cut_weight(W, mask)
        if cw > best_weight:
            best_weight = cw
            best_mask = mask.copy()
    return best_mask, best_weight

# --- Example usage & CSV output ---
if __name__ == '__main__':
    filenames = ['set1/g1.rud']  # extend as needed
    results = []
    for fname in filenames:
        n, W = load_graph(fname)
        rand_mask, rand_w = randomized_heuristic(n, W, trials=100)
        gr_mask, gr_w = greedy_max_cut(n, W)
        semi_mask, semi_w = semi_greedy_max_cut(n, W, alpha=0.5)
        ls_mask, ls_w = local_search(n, W, semi_mask.copy())
        grasp_mask, grasp_w = grasp(n, W, max_iter=50, alpha=0.5)
        results.append([fname, n, int(rand_w), int(gr_w), int(semi_w), int(ls_w), int(grasp_w)])
    # write CSV
    with open('results.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['File','n','Rand','Greedy','Semi','Local','GRASP'])
        writer.writerows(results)
