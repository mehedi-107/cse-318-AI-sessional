import time
import matplotlib.pyplot as plt
from backend import board, minimax, B, R, E

# Settings
time_limits = [3, 4, 5, 8, 10]
time_labels = ['3s', '4s', '5s', '8s', '10s']
NUM_GAMES = 5
DEPTH_A = 3  # Red player
DEPTH_B = 4  # Blue player

def play_game(time_limit):
    game = board(9, 6)
    current_player = R
    while not game.is_game_over():
        start_time = time.perf_counter()
        if current_player == R:
            _, move = minimax(
                game, DEPTH_A, R, R, True,
                -float('inf'), float('inf'),
                start_time, time_limit, None
            )
        else:
            _, move = minimax(
                game, DEPTH_B, B, B, True,
                -float('inf'), float('inf'),
                start_time, time_limit, None
            )
        game = game.make_move(move[0], move[1], current_player)
        current_player ^= 1
    return game.get_winner()

# Collect win stats
wins_A = []
wins_B = []

for tl in time_limits:
    wins_r = wins_b = 0
    for _ in range(NUM_GAMES):
        winner = play_game(tl)
        if winner == R:
            wins_r += 1
        else:
            wins_b += 1
    wins_A.append(wins_r)
    wins_B.append(wins_b)

# Plotting
x = range(len(time_labels))
width = 0.35
plt.figure(figsize=(8, 5))
plt.bar([i - width/2 for i in x], wins_A, width, label=f'Depth {DEPTH_A} wins')
plt.bar([i + width/2 for i in x], wins_B, width, label=f'Depth {DEPTH_B} wins')
plt.xticks(x, time_labels)
plt.xlabel('Time Limit')
plt.ylabel(f'Wins (out of {NUM_GAMES})')
plt.title(f'Depth {DEPTH_A} vs Depth {DEPTH_B} at Different Time Limits')
plt.legend()
plt.tight_layout()
plt.savefig('depth_comparison_wins.png')
plt.close()

# Console summary
for label, a, b in zip(time_labels, wins_A, wins_B):
    print(f'{label}: Depth {DEPTH_A} wins = {a}, Depth {DEPTH_B} wins = {b}')
print('Graph saved as depth_comparison_wins.png')
