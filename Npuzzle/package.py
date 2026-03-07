def count_inversions(arr):
        if len(arr) <= 1:
            return 0, arr
        
        mid = len(arr) // 2
        left_inv, left = count_inversions(arr[:mid])
        right_inv, right = count_inversions(arr[mid:])
        merge_inv, merged = merge_and_count(left, right)
        return left_inv + right_inv + merge_inv, merged

def merge_and_count(left, right):
    result = []
    i = j = inv_count = 0
    
    while i < len(left) and j < len(right):
        if left[i] <= right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1
            inv_count += len(left) - i
    
    result.extend(left[i:])
    result.extend(right[j:])
    return inv_count, result

def inversion_count(arr):
        count, _ = count_inversions(arr)
        return count

def hamming_distance(config: list[list[int]]) -> int:
    cnt = 0
    n = len(config)
    for i in range(n):
        for j in range(n):
            if config[i][j]!=0 and config[i][j] != i * n + j + 1:
                cnt+=1
    return cnt 

def manhattan_distance(config: list[list[int]]) -> int:
    cnt = 0
    n = len(config)
    for i in range(n):
        for j in range(n):
            if config[i][j] != 0:
                # te = cnt
                cnt+=abs((config[i][j]-1)//n - i) + abs((config[i][j]-1)%n - j)
                # print(cnt-te)
    return cnt

def euclidean_distance(config: list[list[int]]) -> float:
    cnt = 0
    n = len(config)
    for i in range(n):
        for j in range(n):
            if config[i][j] != 0:
                # print(((i - config[i][j] // n) ** 2 + (j - (config[i][j]-1) % n) ** 2))
                cnt += ((i - (config[i][j]-1) // n) ** 2 + (j - (config[i][j]-1) % n) ** 2)**0.5
    return cnt

def linear_conflict(config: list[list[int]]) -> int:
    cnt = manhattan_distance(config)
    conflict_count = 0
    n = len(config)
    for i in range(n):
        row = []
        for val in range(n):
            if config[i][val] != 0 and (config[i][val] - 1) // n == i:
                row.append(config[i][val])
        conflict_count += inversion_count(row)
    
    for j in range(n):
        col = []
        for val in range(n):
            if config[val][j] != 0 and (config[val][j] - 1) % n == j:
                col.append(config[val][j])
        conflict_count += inversion_count(col)
    return cnt + 2 * conflict_count


if __name__ == "__main__":
    config = [[7, 2, 4], [6, 0, 5], [8, 3,1]]
    print("Hamming Distance:", hamming_distance(config))
    print("Manhattan Distance:", manhattan_distance(config))
    print("Euclidean Distance:", euclidean_distance(config))
    print("Linear Conflict:", linear_conflict(config))
