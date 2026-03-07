import random
import csv
def cut_weight(adj, X, Y):
    weight = 0
    for u in X:
        for v, w in adj[u]:
            if v in Y:
                weight += w
    return weight


def randomized_heuristic(num_nodes,adj, n):
    total_cut_weight= 0
    best_cut_X = set()
    best_cut_Y = set()
    weight = -1
    for i in range(n):
        X = set()
        Y = set()
        for v in range(num_nodes):
            if random.random() >= 0.5:
                X.add(v)
            else:
                Y.add(v)
        cut = cut_weight(adj, X, Y)
        if cut > weight:
            weight = cut
            best_cut_X = X
            best_cut_Y = Y
        total_cut_weight += cut
    return best_cut_X, best_cut_Y, total_cut_weight // n


def single_weight(adj, z, X):
    weight = 0
    for u, w in adj[z]:
        if u in X:
            weight += w
    return weight


def greedy_max_cut(num_nodes, adj):
    X = set()
    Y = set()
    u = -1
    v = -1
    weight = -1
    for i in range(num_nodes):
        for j,w in adj[i]:
            if w > weight:
                weight = w
                u = i
                v = j
    X.add(u)
    Y.add(v)
    for i in range(num_nodes):
        if i != u and i != v:
            wx = single_weight(adj, i, Y)
            wy = single_weight(adj, i, X)
            if wx > wy:
                X.add(i)
            else:
                Y.add(i)
    cut = cut_weight(adj, X, Y)
    return X, Y, cut


def local_search(num_nodes, adj, S, Sp):
    iter_cnt = 0
    while True:
        delta = [0] * num_nodes
        for i in range(num_nodes):
            ws = single_weight(adj, i, S)
            wsp = single_weight(adj, i, Sp)
            if i in S:
                delta[i] = ws - wsp
            else:
                delta[i] = wsp - ws
        mx = max(delta)
        index = 0
        for i in range(num_nodes):
            if delta[i] == mx:
                index = i
        if mx > 0:
            if index in S:
                Sp.add(index)
                S.remove(index)
            else:
                S.add(index)
                Sp.remove(index)
            iter_cnt += 1
        else:
            break
    return S, Sp, iter_cnt


def semi_greedy_max_cut(node_nums, adj, alpha):
    X = set()
    Y = set()
    U = set()
    for i in range(node_nums):
        U.add(i)
    deltax = {}
    deltay = {}
    for i in range(node_nums):
        deltax[i]=0
        deltay[i]=0
    wmin = 0
    wmax = 0
    while len(U) > 0:
        wmin = min(min(deltax.values()), min(deltay.values()))
        wmax = max(max(deltax.values()), max(deltay.values()))
        meu = wmin + alpha * (wmax - wmin)
        RCL = []
        for v in U:
            if  max(deltax[v], deltay[v]) >= meu:
                RCL.append(v)
        if len(RCL) == 0:
            break
        random_vertex = random.choice(RCL)
        wx_rand = deltax[random_vertex]
        wy_rand = deltay[random_vertex]
        if wx_rand >= wy_rand:
            Y.add(random_vertex)
            for u, w in adj[random_vertex]:
                if u in U:
                    deltay[u] += w      
        else:
            X.add(random_vertex)
            for u, w in adj[random_vertex]:
                if u in U:
                    deltax[u] += w
        U.remove(random_vertex)
        temp1 = {}
        temp2 = {}
        for i in range(num_nodes):
            if i in U:
                temp1[i] = deltax[i]
                temp2[i] = deltay[i]
        deltax = temp1
        deltay = temp2
    cut = cut_weight(adj, X, Y)
    return X, Y, cut


def grasp(max_iter, num_nodes, adj, heuristic_no = 3, alpha = 0.5):
    Xsolve = set()
    Ysolve = set()
    solve_cut_weight = -1
    for i in range(max_iter):
        if heuristic_no == 1:
            X, Y, _ = randomized_heuristic(num_nodes, adj, 1000)
        elif heuristic_no == 2:
            X, Y, _ = greedy_max_cut(num_nodes, adj)
        else:
            X, Y, _ = semi_greedy_max_cut(num_nodes, adj, alpha)
        X, Y, _ = local_search(num_nodes, adj, X, Y)
        cut = cut_weight(adj, X, Y)
        if i == 0 or cut > solve_cut_weight:
            Xsolve = X
            Ysolve = Y
            solve_cut_weight = cut
    return Xsolve, Ysolve, solve_cut_weight


if __name__ == "__main__":
    results = []
    for ab in range(1,55):
        known_best = ["N/A", 12078, 12084, 12077, "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", 627, 621, 645, 3187, 3169, 3172, "N/A", "N/A", "N/A", "N/A", "N/A", 14123, 14129, 14131, "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", 1560, 1537, 1541, 8000, 7996, 8009, "N/A", "N/A", "N/A", "N/A", "N/A", 7027, 7022, 7020, "N/A", "N/A", 6000, 6000, 5988,"N/A", "N/A", "N/A", "N/A", "N/A"]
        file_name = "set1/g" + str(ab) + ".rud"
        print("File: ", file_name)
        with open(file_name, "r") as f:
            lines = f.readlines()
        num_nodes, num_edges = map(int, lines[0].split())
        edges = []
        for i in range(num_edges):
            u, v, w = map(int, lines[i + 1].split())
            edges.append((u, v, w))
        for i in range(num_edges):
            u, v, w = edges[i]
            edges[i] = (u - 1, v - 1, w)
        adj = {i: [] for i in range(num_nodes)}
        for u, v, w in edges:
            adj[u].append((v, w))
            adj[v].append((u, w))
        rand_x, rand_y, randomized_avg = randomized_heuristic(num_nodes, adj, 5)
        greedy_x, greedy_y,greedy_cut = greedy_max_cut(num_nodes, adj)
        semi_greedy_x, semi_greedy_y, semi_val = semi_greedy_max_cut(num_nodes, adj, 0.8)
        local_avg = 0
        for i in range(5):
            S, Sp, local_iter = local_search(num_nodes, adj, semi_greedy_x, semi_greedy_y)
            local_avg += cut_weight(adj, S, Sp)
            semi_greedy_x, semi_greedy_y, _ = semi_greedy_max_cut(num_nodes, adj, 0.8)
            
        local_avg //= 5
        grasp_iters = 5
        alphas = [0.5, 0.75, 0.9, 0.95, 0.99]
        grasp_vals = []
        best_alpha = 0.0
        best_val = 0
        for alpha in alphas:
            X, Y, grashp_best = grasp(grasp_iters, num_nodes, adj, 3, alpha)
            grasp_vals.append(grashp_best)
            if grashp_best > best_val:
                best_val = grashp_best
                best_alpha = alpha
        grasp_best_alpha = str(best_val) + "(" + str(best_alpha) + ")"
        
        results.append([
            "G" + str(ab),
            num_nodes, num_edges,
            int(randomized_avg),
            greedy_cut,
            str(semi_val) + "(0.85)",
            5,
            local_avg,
            5,
            grasp_best_alpha,
            known_best[ab]
        ])

    with open("2105052.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "Name", "|V| or n", "|E| or m",
            "Simple Randomized or Randomized-1",
            "Simple Greedy or Greedy-1",
            "Semi-greedy-1",
            "Simple local or local-1 - No. of iterations",
            "Simple local or local-1 - Average value",
            "GRASP-1 - No. of iterations",
            "GRASP-1 - Best value",
            "Known best solution"
        ])
        writer.writerows(results)
