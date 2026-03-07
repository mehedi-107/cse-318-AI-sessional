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
        print("Iteration: ", iter_cnt)
    return S, Sp, iter_cnt