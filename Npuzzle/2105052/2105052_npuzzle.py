from queue import PriorityQueue
import package as pkg
import os

class board:
    def __init__(self, n : int, config : list[list[int]],heuristic_func, cost: int = 0, parent = None) -> None:
        self.config = config
        self.n = n
        self.cost = cost
        self.parent = parent
        self.heuristic_func = heuristic_func    

    def get_heuristic_cost(self):
        return self.heuristic_func(self.config)
    
    def get_blank_position(self)-> tuple[int,int]:
        for i in range(self.n):
            for j in range(self.n):
                if self.config[i][j] == 0:
                    return (i, j)
        return (-1,-1)
    
    def get_valid_moves(self)-> list[tuple[int,int]]:
        valid_moves = []
        x,y = self.get_blank_position()
        # print("x=",x,"y=",y)
        temp = [(x-1,y),(x,y-1),(x+1,y),(x,y+1)]
        # print("temp = ", temp)
        for ele in temp:
            if 0 <= ele[0] < self.n and 0 <= ele[1] < self.n:
                valid_moves.append(ele)
        return valid_moves
    
    def make_move(self,move):
        new_config = [row.copy() for row in self.config] 
        black_pos = self.get_blank_position()
        new_config[black_pos[0]][black_pos[1]], new_config[move[0]][move[1]] = new_config[move[0]][move[1]], new_config[black_pos[0]][black_pos[1]]
        return board(self.n, new_config,self.heuristic_func,self.cost+1, parent=self)
    
    def get_priority_value(self)-> int:
        return self.cost + self.get_heuristic_cost()
    
    def is_solved(self)-> bool:
        for i in range(self.n):
            for j in range(self.n):
                if (i, j) == (self.n-1, self.n-1):
                    if self.config[i][j] != 0:
                        return False
                else:
                    if self.config[i][j] != i*self.n + j + 1:
                        return False
        return True
    
    def __eq__(self, other):
        if not isinstance(other, board):
            return False
        return self.config == other.config and self.n == other.n
    
    def __lt__(self,other):
        return self.get_priority_value() < other.get_priority_value()
    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.config))
    


class NPuzzle:
    def __init__(self, n: int,heuristic_func, config: list[list[int]]) -> None:
        self.n = n
        self.current_state = board(n,config,heuristic_func,0,None)
        self.open_list = PriorityQueue()
        self.open_list.put((self.current_state.get_priority_value(), self.current_state))
        self.close_list = set()
        self.heuristic_func = heuristic_func
    
    def is_solvable(self, config: list[int]):
        arr = []
        zero_pos = 0 
        for i in range(self.n):
            for j in range(self.n):
                if config[i][j] != 0:
                    arr.append(config[i][j])
                if config[i][j] == 0:
                    zero_pos = self.n - i
        cnt = pkg.inversion_count(arr)
        # print(cnt)
        # print(zero_pos)
        if self.n%2 == 1 and cnt%2 == 0:
            return True
        elif self.n%2 == 0:
            if zero_pos%2 == 0 and cnt%2 == 1:
                return True
            elif zero_pos%2 == 1 and cnt%2 == 0:
                return True
            else:
                return False
        else:
            return False

    
    
    def solve(self):
        if not self.is_solvable(self.current_state.config):
            return None
        while not self.open_list.empty():
            (priority, board) = self.open_list.get()
            # print("p=",priority)
            self.close_list.add(board)
            if board.is_solved():
                result = []
                while board:
                    result.append(board)
                    board = board.parent
                result = result[::-1]
                return result
            valid_moves = board.get_valid_moves()
            for i in range(len(valid_moves)):
                new_board = board.make_move(valid_moves[i])
                if new_board not in self.close_list:
                    self.open_list.put((new_board.get_priority_value(), new_board))
        return None
    

if __name__ == "__main__":
    n = int(input("Enter the size of the puzzle: "))
    config = []
    print("Enter the puzzle config: ")
    for _ in range(n):
        row = list(map(int, input().strip().split()))
        config.append(row)
    puzzle = NPuzzle(n, pkg.manhattan_distance, config)
    puzzle2 = NPuzzle(n, pkg.hamming_distance, config.copy())
    puzzle3 = NPuzzle(n, pkg.linear_conflict, config.copy())
    puzzle4 = NPuzzle(n, pkg.euclidean_distance, config.copy())
    result = puzzle.solve()
    result2 = puzzle2.solve()
    result3 = puzzle3.solve()
    result4 = puzzle4.solve()

    print("Results:")
    if result:
        print("Heuristic function: manhattan distance")
        print("Minimum number of moves: " + str(len(result)-1))
        print("Number of node expanded: " + str(len(puzzle.close_list)-1))
        print("Number of node explored: " + str(puzzle.open_list.qsize() +len(puzzle.close_list)))
        for brd in result:
            for i in brd.config:
                for j in i:
                    print(j,end=" ")
                print("")
            print("")
    else:
        print("Unsolvable puzzle")
    
    if result2:
        print("Heuristic function: hamming distance")
        print("Minimum number of moves: " + str(len(result2)-1))
        print("Number of node expanded: " + str(len(puzzle2.close_list)-1))
        print("Number of node explored: " + str(puzzle2.open_list.qsize() +len(puzzle2.close_list)))
        for brd in result2:
            for i in brd.config:
                for j in i:
                    print(j,end=" ")
                print("")
            print("")

    else:
        print("unsolvable puzzle")
    
    if result3:
        print("Heuristic function: linear conflict")
        print("Minimum number of moves: " + str(len(result3)-1))
        print("Number of node expanded: " + str(len(puzzle3.close_list)-1))
        print("Number of node explored: " + str(puzzle3.open_list.qsize() +len(puzzle3.close_list)))
        for brd in result3:
            for i in brd.config:
                for j in i:
                    print(j,end=" ")
                print("")
            print("")
    
    else:
        print("unsolvable puzzle")
    
    if result4:
        print("Heuristic function: euclidean distance")
        print("Minimum number of moves: " + str(len(result4)-1))
        print("Number of node expanded: " + str(len(puzzle4.close_list)-1))
        print("Number of node explored: " + str(puzzle4.open_list.qsize() +len(puzzle4.close_list)))
        for brd in result4:
            for i in brd.config:
                for j in i:
                    print(j,end=" ")
                print("")
            print("")
    else:
        print("unsolvable puzzle")


    # if os.path.exists("input.txt"):
    #     N = []
    #     config = []
    #     with open("input.txt", "r") as f:
    #         n = int(f.readline())
    #         configg = []
    #         for _ in range(n):
    #             row = list(map(int, f.readline().strip().split()))
    #             config.append(row)
    #     puzzle = NPuzzle(n, pkg.manhattan_distance, config)
    #     puzzle2 = NPuzzle(n, pkg.hamming_distance, config.copy())
    #     puzzle3 = NPuzzle(n, pkg.linear_conflict, config.copy())
    #     puzzle4 = NPuzzle(n, pkg.euclidean_distance, config.copy())
    #     result = puzzle.solve()
    #     result2 = puzzle2.solve()
    #     result3 = puzzle3.solve()
    #     result4 = puzzle4.solve()
    #     with open("output.txt", "w") as f:
    #         if result:
    #             f.write("Heuristic function: manhattan distance\n")
    #             f.write("Minimum number of moves: " + str(len(result)-1) + "\n")
    #             f.write("Number of node expanded: " + str(len(puzzle.close_list)-1) + "\n")
    #             f.write("Number of node explored: " + str(puzzle.open_list.qsize() +len(puzzle.close_list))+"\n")
    #             for brd in result:
    #                 for i in brd.config:
    #                     for j in i:
    #                         f.write(str(j)+" ")
    #                     f.write("\n")
    #                 f.write("\n")
    #         else:
    #             f.write("unsolvable puzzle\n")

    #         if result2:
    #             f.write("Heuristic function: hamming distance\n")
    #             f.write("Minimum number of moves: " + str(len(result2)-1) + "\n")
    #             f.write("Number of node expanded: " + str(len(puzzle2.close_list)-1) + "\n")
    #             f.write("Number of node explored: " + str(puzzle2.open_list.qsize() +len(puzzle2.close_list))+"\n")
    #             for brd in result2:
    #                 for i in brd.config:
    #                     for j in i:
    #                         f.write(str(j)+" ")
    #                     f.write("\n")
    #                 f.write("\n")
    #         else:
    #             f.write("unsolvable puzzle\n")
            
    #         if result3:
    #             f.write("Heuristic function: linear conflict\n")
    #             f.write("Minimum number of moves: " + str(len(result3)-1) + "\n")
    #             f.write("Number of node expanded: " + str(len(puzzle3.close_list)-1) + "\n")
    #             f.write("Number of node explored: " + str(puzzle3.open_list.qsize() +len(puzzle3.close_list))+"\n")
    #             for brd in result3:
    #                 for i in brd.config:
    #                     for j in i:
    #                         f.write(str(j)+" ")
    #                     f.write("\n")
    #                 f.write("\n")
    #         else:
    #             f.write("unsolvable puzzle\n")

    #         if result4:
    #             f.write("Heuristic function: euclidean distance\n")
    #             f.write("Minimum number of moves: " + str(len(result4)-1) + "\n")
    #             f.write("Number of node expanded: " + str(len(puzzle4.close_list)-1) + "\n")
    #             f.write("Number of node explored: " + str(puzzle4.open_list.qsize() +len(puzzle4.close_list))+"\n")
    #             for brd in result4:
    #                 for i in brd.config:
    #                     for j in i:
    #                         f.write(str(j)+" ")
    #                     f.write("\n")
    #                 f.write("\n")
    #         else:
    #             f.write("unsolvable puzzle\n")


    

            

        

# 3 
# 1 2 3 
# 0 4 6 
# 8 5 7


# 3 
# 1 2 3
# 4 5 6
# 8 7 0

# 3 
# 1 2 3 
# 0 4 6 
# 7 5 8


# 3
# 0 1 3 
# 4 2 5 
# 7 8 6