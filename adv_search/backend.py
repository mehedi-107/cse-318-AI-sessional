import math
import copy
import random
import time
from queue import Queue


B = 0
R = 1
E = -1

class board:
    def __init__(self, row, col, config=None):
        self.row = row
        self.col = col
        if config is not None:
            self.config = config
        else:
            self.config = [[(E, 0) for _ in range(col)] for _ in range(row)]

    def print_config(self):
        for i in range(self.row):
            for j in range(self.col):
                print(self.config[i][j], end=" ")
            print("")
    
    def get_critical_mass(self, i, j):
        c_mass = 4
        if i == 0 or i == self.row - 1:
            c_mass -= 1
        if j == 0 or j == self.col - 1:
            c_mass -= 1
        return c_mass
    
    def is_game_over(self):
        r_cnt = 0
        b_cnt = 0
        for i in range(self.row):
            for j in range(self.col):
                if self.config[i][j][0] == R:
                    r_cnt += 1
                elif self.config[i][j][0] == B:
                    b_cnt += 1
        if (r_cnt == 0 and b_cnt > 1) or (b_cnt == 0 and r_cnt > 1):
            return True
        return False
    
    def get_winner(self):
        r_cnt = 0
        b_cnt = 0
        for i in range(self.row):
            for j in range(self.col):
                if self.config[i][j][0] == R:
                    r_cnt += 1
                elif self.config[i][j][0] == B:
                    b_cnt += 1
        if b_cnt == 0 and r_cnt > 1:
            return R
        elif r_cnt == 0 and b_cnt > 1:
            return B
        return E
    
    def is_valid_move(self, i, j, player):
        if i >= 0 and i < self.row and j >= 0 and j < self.col and (self.config[i][j][0] == E or self.config[i][j][0] == player):
            return True
        return False

    def all_legal_moves(self, player):
        moves = []
        for i in range(self.row):
            for j in range(self.col):
                if self.is_valid_move(i, j, player):
                    moves.append((i, j))
        return moves

    def get_cell_valid_moves(self, i, j):
        moves = []
        if i + 1 < self.row:
            moves.append((i + 1, j))
        if i - 1 >= 0:
            moves.append((i - 1, j))
        if j + 1 < self.col:
            moves.append((i, j + 1))
        if j - 1 >= 0:
            moves.append((i, j - 1))
        return moves
    
    def make_move(self, i, j, player):
        new_board = copy.deepcopy(self)
        _, cnt = new_board.config[i][j]
        new_board.config[i][j] = (player, cnt + 1)
        q = Queue()
        if cnt + 1 >= new_board.get_critical_mass(i, j):
            q.put((i, j))
        
        while q.empty() == False and new_board.is_game_over() == False:
            x, y = q.get()
            _, cnt = new_board.config[x][y]
            if cnt < new_board.get_critical_mass(x, y):
                continue
            c_mass = new_board.get_critical_mass(x, y)
            if cnt > c_mass:
                new_board.config[x][y] = (player, cnt - c_mass)
            else:
                new_board.config[x][y] = (E, 0)
            for x_val, y_val in new_board.get_cell_valid_moves(x, y):
                _, ncnt = new_board.config[x_val][y_val]
                new_board.config[x_val][y_val] = (player, ncnt + 1)
                if ncnt + 1 >= new_board.get_critical_mass(x_val, y_val):
                    q.put((x_val, y_val))

        return new_board


    def orb_cnt_diff(self, player):
        cnt = 0
        for i in range(self.row):
            for j in range(self.col):
                cell_player, cell_cnt  = self.config[i][j]
                if cell_player == player:
                    cnt += cell_cnt
                elif cell_player != E:
                    cnt -= cell_cnt
        return cnt

    def close_to_explosion(self, player):
        cnt = 0.0
        for i in range(self.row):
            for j in range(self.col):
                cell_player, cell_cnt = self.config[i][j]
                if cell_player == player:
                    cnt += cell_cnt / self.get_critical_mass(i, j)
                elif cell_player != E:
                    cnt -= cell_cnt / self.get_critical_mass(i, j)
        return cnt
    
    def corner_or_edge_advantage(self, player):
        cnt = 0
        for i in range(self.row):
            for j in range(self.col):
                cell_player, _ = self.config[i][j]
                if cell_player == player:
                    if (i == 0 or i == self.row - 1) or (j == 0 or j == self.col - 1):
                        cnt += 1
                elif cell_player != E:
                    if (i == 0 or i == self.row - 1) or (j == 0 or j == self.col - 1):
                        cnt -= 1
        return cnt
    
    def ready_to_explode(self, player):
        cnt = 0
        for i in range(self.row):
            for j in range(self.col):
                cell_player, cell_cnt = self.config[i][j]
                if cell_player == player and cell_cnt +1 == self.get_critical_mass(i, j):
                    cnt += cell_cnt
                elif cell_player != E and cell_cnt + 1 >= self.get_critical_mass(i, j):
                    cnt -=  cell_cnt
        return cnt
    
    def hybrid_heuristic(self, player):
        orb_diff = self.orb_cnt_diff(player)
        explosion_risk = self.close_to_explosion(player)
        edge_advantage = self.corner_or_edge_advantage(player)
        ready_to_explode = self.ready_to_explode(player)
        w1 = 1.0
        w2 = 3.5
        w3 = 2.0
        w4 = 4.0
        return w1 * orb_diff + w2 * explosion_risk + w3 * edge_advantage + w4 * ready_to_explode


    def evaluate(self, player, selected_heuristic=None):
        if selected_heuristic == 1:
            return self.orb_cnt_diff(player)
        elif selected_heuristic == 2:
            return self.close_to_explosion(player)
        elif selected_heuristic == 3:
            return self.corner_or_edge_advantage(player)
        elif selected_heuristic == 4:
            return self.ready_to_explode(player)
        elif selected_heuristic == 5:
            return self.hybrid_heuristic(player)
        if selected_heuristic is None:
            return self.hybrid_heuristic(player)



def minimax(curr_board, depth, curr_player, root_player, maximizing, alpha, beta,start_time, time_limit, selected_heuristic=None):
    if depth == 0 or curr_board.is_game_over():
        return curr_board.evaluate(root_player, selected_heuristic), None

    moves = curr_board.all_legal_moves(curr_player)
    best_move = random.choice(moves)
    random.shuffle(moves)
    if maximizing:
        max_val = -math.inf
        for move in moves:
            next_board = curr_board.make_move(move[0], move[1], curr_player)
            other_player = curr_player ^ 1 
            val, _ = minimax(next_board, depth - 1, other_player, root_player, False, alpha, beta, start_time, time_limit, selected_heuristic)
            if val > max_val:
                max_val, best_move = val, move
            alpha = max(alpha, val)
            if alpha >= beta:
                break
            if start_time is not None and time_limit is not None and time.time() - start_time > time_limit:
                # print("Time limit exceeded, returning best move found so far.")
                return max_val, best_move
        return max_val, best_move
    else:
        min_val = math.inf
        for move in moves:
            next_board = curr_board.make_move(move[0], move[1], curr_player)
            other_player = curr_player ^ 1
            val, _ = minimax(next_board, depth - 1, other_player, root_player, True, alpha, beta, start_time, time_limit, selected_heuristic)
            if val < min_val:
                min_val, best_move = val, move
            beta = min(beta, val)
            if alpha >= beta:
                break
            if start_time is not None and time_limit is not None and time.time() - start_time > time_limit:
                # print("Time limit exceeded, returning best move found so far.")
                return min_val, best_move
        return min_val, best_move
