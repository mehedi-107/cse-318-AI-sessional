import time
import math
import random
from backend import board, B, R, E, minimax

DEPTH = 4
ROWS = 9
COLS = 6
TIME_LIMIT = 5.0 
GAMESTATE_FILE = "gamestate.txt"

def save_game_state(game_board, player):
    with open(GAMESTATE_FILE, 'w') as f:
        f.write(f"{player} Move:\n")
        for i in range(game_board.row):
            for j in range(game_board.col):
                if game_board.config[i][j][0] == R:
                    f.write(f"{game_board.config[i][j][1]}R ")
                elif game_board.config[i][j][0] == B:
                    f.write(f"{game_board.config[i][j][1]}B ")
                else:
                    f.write("0 ")
            f.write("\n")

def get_human_move(game_board, player):
    while True:
        move = input(f"Player {player} Move (row col): ")
        i, j = map(int, move.split())
        if game_board.is_valid_move(i, j, player):
            return (i, j)
        else:
            print("Invalid move. Try again.")               

def get_ai_move(game_board, player, heuristic=5):
    _, move = minimax(game_board, DEPTH, player,player, True, -math.inf, math.inf, time.time(), TIME_LIMIT, heuristic)
    return move

def get_random_move(game_board, player):
    valid_moves = [(i, j) for i in range(game_board.row) for j in range(game_board.col) if game_board.is_valid_move(i, j, player)]
    if valid_moves:
        return random.choice(valid_moves)
    return None

def main():
    game_board = board(ROWS, COLS)
    current_player = R 
    mode = input("Choose mode (1 for Human vs AI, 2 for AI vs AI), 3 for Random vs AI: ")
    if mode == '1':
        print("Select Heuristic Function:")
        print("1. Orb count difference")
        print("2. Close to explosion")
        print("3. Corner/edge advantage")
        print("4. Ready to explode")
        print("5. Hybrid heuristic")
        heuristic = input("Enter your choice (1-5): ")
        if heuristic not in {'1', '2', '3', '4', '5'}:
            print("Invalid choice, defaulting to Hybrid heuristic.")
            heuristic = '5'
        while not game_board.is_game_over():
            game_board.print_config()
            print(f"Current Player: {'R' if current_player == R else 'B'}")
            if current_player == R:
                move = get_human_move(game_board, current_player)
            else:
                print("AI is thinking...")
                move = get_ai_move(game_board, current_player, int(heuristic))
            game_board = game_board.make_move(move[0], move[1], current_player)
            if current_player == R:
                save_game_state(game_board, "Human")
            else:
                save_game_state(game_board, "AI")
            current_player ^= 1
        winner = game_board.get_winner()
        game_board.print_config()
        if winner == R:
            print("You win!")
        else:
            print("AI wins!")
    elif mode == '2':
        print("Choose heuristic for AI:")
        print("1. Orb count difference")
        print("2. Close to explosion")
        print("3. Corner/edge advantage")
        print("4. Ready to explode")
        print("5. Hybrid heuristic")
        heuristic_r = input("Enter your choice (1-5) for AI(R): ")
        if heuristic_r not in {'1', '2', '3', '4', '5'}:
            print("Invalid choice, defaulting to Hybrid heuristic.")
            heuristic_r = '5'
        
        print("Choose heuristic for AI(B):")
        heuristic_b = input("Enter your choice (1-5) for AI(B): ")
        if heuristic_b not in {'1', '2', '3', '4', '5'}:
            print("Invalid choice, defaulting to Hybrid heuristic.")
            heuristic_b = '5'

        while not game_board.is_game_over():
            game_board.print_config()
            print(f"Current Player: {'R' if current_player == R else 'B'}")
            if current_player == R:
                print("AI(R) is thinking...")
                move = get_ai_move(game_board, current_player, int(heuristic_r))
            else:
                print("AI(B) is thinking...")
                move = get_ai_move(game_board, current_player, int(heuristic_b))
            game_board = game_board.make_move(move[0], move[1], current_player)
            if current_player == R:
                save_game_state(game_board, "AI(R)")
            else:
                save_game_state(game_board, "AI(B)")
            current_player ^= 1
            # time.sleep(0.5)
        winner = game_board.get_winner()
        game_board.print_config()
        if winner == R:
            print("AI R wins!")
        else:
            print("AI B wins!")
    elif mode == '3':
        print("Select Heuristic Function:")
        print("1. Orb count difference")
        print("2. Close to explosion")
        print("3. Corner/edge advantage")
        print("4. Ready to explode")
        print("5. Hybrid heuristic")
        heuristic = input("Enter your choice (1-5): ")
        if heuristic not in {'1', '2', '3', '4', '5'}:
            print("Invalid choice, defaulting to Hybrid heuristic.")
            heuristic = '5'
        while not game_board.is_game_over():
            game_board.print_config()
            print(f"Current Player: {'R' if current_player == R else 'B'}")
            if current_player == R:
                print("Random Player is thinking...")
                move = get_random_move(game_board, current_player)
            else:
                print("AI is thinking...")
                move = get_ai_move(game_board, current_player, int(heuristic))
            game_board = game_board.make_move(move[0], move[1], current_player)
            if current_player == R:
                save_game_state(game_board, "Random")
            else:
                save_game_state(game_board, "AI")
            current_player ^= 1
        winner = game_board.get_winner()
        game_board.print_config()
        if winner == R:
            print("Random Player wins!")
        else:
            print("AI wins!")
            
            


if __name__ == "__main__":
    main()
