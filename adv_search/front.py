import pygame
import sys
import math
import time
from backend import board, minimax, B, R, E

# Game states
MENU = 0
PLAYING = 1
GAME_OVER = 2

# Game modes
HUMAN_VS_AI = 0
AI_VS_AI = 1

class GameUI:
    def __init__(self):
        # Initialize game with backend default size
        self.game = board(9, 6)
        self.ROWS, self.COLS = self.game.row, self.game.col
        
        # UI Configuration
        self.CELL_SIZE = 80
        self.MARGIN = 5
        self.WIDTH = self.COLS * self.CELL_SIZE + (self.COLS + 1) * self.MARGIN
        self.HEIGHT = self.ROWS * self.CELL_SIZE + (self.ROWS + 1) * self.MARGIN + 100
        self.DEPTH = 4
        
        # Game state
        self.state = MENU
        self.game_mode = HUMAN_VS_AI
        self.current_player = R
        self.ai_thinking = False
        self.winner = None
        
        # Initialize pygame
        pygame.init()
        self.screen = pygame.display.set_mode((self.WIDTH, self.HEIGHT))
        pygame.display.set_caption("Chain Reaction")
        self.font = pygame.font.SysFont(None, 36)
        self.big_font = pygame.font.SysFont(None, 48)
        self.small_font = pygame.font.SysFont(None, 24)
        
        # Colors
        self.WHITE = (255, 255, 255)
        self.BLACK = (0, 0, 0)
        self.RED = (255, 0, 0)
        self.BLUE = (0, 0, 255)
        self.GREEN = (0, 200, 0)
        self.GRAY = (200, 200, 200)
        self.DARK_GRAY = (100, 100, 100)
        self.LIGHT_BLUE = (173, 216, 230)
        
        # UI elements
        self.buttons = {}
        self.create_buttons()
    
    def create_buttons(self):
        # Menu buttons
        button_width = 200
        button_height = 50
        center_x = self.WIDTH // 2 - button_width // 2
        
        self.buttons['human_vs_ai'] = pygame.Rect(center_x, 200, button_width, button_height)
        self.buttons['ai_vs_ai'] = pygame.Rect(center_x, 270, button_width, button_height)
        self.buttons['start'] = pygame.Rect(center_x, 340, button_width, button_height)
        
        # Game buttons
        self.buttons['ai_move'] = pygame.Rect(self.WIDTH - 120, self.HEIGHT - 80, 110, 35)
        self.buttons['restart'] = pygame.Rect(10, self.HEIGHT - 80, 80, 35)
        self.buttons['menu'] = pygame.Rect(100, self.HEIGHT - 80, 80, 35)
    
    def draw_menu(self):
        self.screen.fill(self.WHITE)
        
        # Title
        title_text = self.big_font.render("Chain Reaction", True, self.BLACK)
        title_rect = title_text.get_rect(center=(self.WIDTH // 2, 100))
        self.screen.blit(title_text, title_rect)
        
        # Subtitle
        subtitle_text = self.font.render("Select Game Mode", True, self.DARK_GRAY)
        subtitle_rect = subtitle_text.get_rect(center=(self.WIDTH // 2, 150))
        self.screen.blit(subtitle_text, subtitle_rect)
        
        # Mode buttons
        for mode, rect in [('human_vs_ai', self.buttons['human_vs_ai']), 
                          ('ai_vs_ai', self.buttons['ai_vs_ai'])]:
            color = self.LIGHT_BLUE if (mode == 'human_vs_ai' and self.game_mode == HUMAN_VS_AI) or \
                                     (mode == 'ai_vs_ai' and self.game_mode == AI_VS_AI) else self.GRAY
            pygame.draw.rect(self.screen, color, rect)
            pygame.draw.rect(self.screen, self.BLACK, rect, 2)
            
            text = "Human vs AI" if mode == 'human_vs_ai' else "AI vs AI"
            button_text = self.font.render(text, True, self.BLACK)
            text_rect = button_text.get_rect(center=rect.center)
            self.screen.blit(button_text, text_rect)
        
        # Start button
        pygame.draw.rect(self.screen, self.GREEN, self.buttons['start'])
        pygame.draw.rect(self.screen, self.BLACK, self.buttons['start'], 2)
        start_text = self.font.render("START GAME", True, self.WHITE)
        start_rect = start_text.get_rect(center=self.buttons['start'].center)
        self.screen.blit(start_text, start_rect)
        
        # Instructions
        instructions = [
            "Rules:",
            "• Click on empty cells or your own cells to add orbs",
            "• Cells explode when they reach critical mass",
            "• Explosions convert neighboring cells to your color",
            "• Win by eliminating all opponent orbs"
        ]
        
        y_offset = 420
        for instruction in instructions:
            text = self.small_font.render(instruction, True, self.DARK_GRAY)
            self.screen.blit(text, (20, y_offset))
            y_offset += 25
    
    def draw_game(self):
        self.screen.fill(self.WHITE)
        
        # Draw cells
        for i in range(self.ROWS):
            for j in range(self.COLS):
                x = self.MARGIN + j * (self.CELL_SIZE + self.MARGIN)
                y = self.MARGIN + i * (self.CELL_SIZE + self.MARGIN) + 50
                rect = pygame.Rect(x, y, self.CELL_SIZE, self.CELL_SIZE)
                pygame.draw.rect(self.screen, self.GRAY, rect)
                pygame.draw.rect(self.screen, self.BLACK, rect, 2)
                
                owner, cnt = self.game.config[i][j]
                if cnt > 0:
                    color = self.RED if owner == R else self.BLUE
                    text = self.font.render(str(cnt), True, color)
                    tx = x + (self.CELL_SIZE - text.get_width()) // 2
                    ty = y + (self.CELL_SIZE - text.get_height()) // 2
                    self.screen.blit(text, (tx, ty))
        
        # Game mode and turn info
        mode_text = "Human vs AI" if self.game_mode == HUMAN_VS_AI else "AI vs AI"
        mode_surface = self.font.render(mode_text, True, self.BLACK)
        self.screen.blit(mode_surface, (10, 10))
        
        if self.game_mode == HUMAN_VS_AI:
            if self.current_player == R:
                status_text = "Your turn (Red)" if not self.ai_thinking else "AI thinking..."
            else:
                status_text = "AI turn (Blue)"
        else:
            player_name = "Red AI" if self.current_player == R else "Blue AI"
            status_text = f"{player_name}'s turn"
        
        status_surface = self.font.render(status_text, True, self.BLACK)
        self.screen.blit(status_surface, (10, self.HEIGHT - 45))
        
        # Control buttons
        # Restart button
        pygame.draw.rect(self.screen, self.GRAY, self.buttons['restart'])
        pygame.draw.rect(self.screen, self.BLACK, self.buttons['restart'], 2)
        restart_text = self.small_font.render("Restart", True, self.BLACK)
        restart_rect = restart_text.get_rect(center=self.buttons['restart'].center)
        self.screen.blit(restart_text, restart_rect)
        
        # Menu button
        pygame.draw.rect(self.screen, self.GRAY, self.buttons['menu'])
        pygame.draw.rect(self.screen, self.BLACK, self.buttons['menu'], 2)
        menu_text = self.small_font.render("Menu", True, self.BLACK)
        menu_rect = menu_text.get_rect(center=self.buttons['menu'].center)
        self.screen.blit(menu_text, menu_rect)
        
        # AI Move button (only for human vs AI mode when it's human's turn)
        if self.game_mode == HUMAN_VS_AI and self.current_player == R and not self.ai_thinking:
            pygame.draw.rect(self.screen, self.GREEN, self.buttons['ai_move'])
            pygame.draw.rect(self.screen, self.BLACK, self.buttons['ai_move'], 2)
            ai_text = self.small_font.render("AI Move", True, self.WHITE)
            ai_rect = ai_text.get_rect(center=self.buttons['ai_move'].center)
            self.screen.blit(ai_text, ai_rect)
    
    def draw_game_over(self):
        self.draw_game()  # Draw the final board state
        
        # Draw semi-transparent overlay
        overlay = pygame.Surface((self.WIDTH, self.HEIGHT))
        overlay.set_alpha(128)
        overlay.fill(self.BLACK)
        self.screen.blit(overlay, (0, 0))
        
        # Winner message
        if self.winner == R:
            msg = "You Win!" if self.game_mode == HUMAN_VS_AI else "Red AI Wins!"
            color = self.RED
        elif self.winner == B:
            msg = "AI Wins!" if self.game_mode == HUMAN_VS_AI else "Blue AI Wins!"
            color = self.BLUE
        else:
            msg = "Draw!"
            color = self.BLACK
        
        win_text = self.big_font.render(msg, True, color)
        win_rect = win_text.get_rect(center=(self.WIDTH // 2, self.HEIGHT // 2 - 50))
        self.screen.blit(win_text, win_rect)
        
        # Play again prompt
        again_text = self.font.render("Click anywhere to continue", True, self.WHITE)
        again_rect = again_text.get_rect(center=(self.WIDTH // 2, self.HEIGHT // 2 + 20))
        self.screen.blit(again_text, again_rect)
    
    def handle_menu_click(self, pos):
        if self.buttons['human_vs_ai'].collidepoint(pos):
            self.game_mode = HUMAN_VS_AI
        elif self.buttons['ai_vs_ai'].collidepoint(pos):
            self.game_mode = AI_VS_AI
        elif self.buttons['start'].collidepoint(pos):
            self.start_game()
    
    def handle_game_click(self, pos):
        if self.buttons['restart'].collidepoint(pos):
            self.start_game()
        elif self.buttons['menu'].collidepoint(pos):
            self.state = MENU
        elif self.game_mode == HUMAN_VS_AI and self.current_player == R and not self.ai_thinking:
            if self.buttons['ai_move'].collidepoint(pos):
                # AI makes move for human
                _, move = minimax(self.game, self.DEPTH, R, R, True, -math.inf, math.inf)
                if move:
                    self.game = self.game.make_move(move[0], move[1], R)
                    self.current_player = B
            else:
                # Human makes move
                self.handle_cell_click(pos)
    
    def handle_cell_click(self, pos):
        for i in range(self.ROWS):
            for j in range(self.COLS):
                x = self.MARGIN + j * (self.CELL_SIZE + self.MARGIN)
                y = self.MARGIN + i * (self.CELL_SIZE + self.MARGIN) + 50
                rect = pygame.Rect(x, y, self.CELL_SIZE, self.CELL_SIZE)
                if rect.collidepoint(pos) and self.game.is_valid_move(i, j, self.current_player):
                    self.game = self.game.make_move(i, j, self.current_player)
                    self.current_player = B if self.current_player == R else R
                    break
    
    def start_game(self):
        self.game = board(9, 6)  # Reset game
        self.current_player = R
        self.ai_thinking = False
        self.winner = None
        self.state = PLAYING
    
    def update_ai(self):
        if self.state != PLAYING or self.game.is_game_over():
            return
        
        # Handle AI moves
        if self.game_mode == AI_VS_AI:
            if not self.ai_thinking:
                self.ai_thinking = True
                time.sleep(0.5)  # Visual delay for AI vs AI
                _, move = minimax(self.game, self.DEPTH, self.current_player, self.current_player, True, -math.inf, math.inf)
                if move:
                    self.game = self.game.make_move(move[0], move[1], self.current_player)
                self.current_player = B if self.current_player == R else R
                self.ai_thinking = False
        
        elif self.game_mode == HUMAN_VS_AI and self.current_player == B:
            if not self.ai_thinking:
                self.ai_thinking = True
                _, move = minimax(self.game, self.DEPTH, B, B, True, -math.inf, math.inf)
                if move:
                    self.game = self.game.make_move(move[0], move[1], B)
                self.current_player = R
                self.ai_thinking = False
    
    def check_game_over(self):
        if self.game.is_game_over():
            self.winner = self.game.get_winner()
            self.state = GAME_OVER
    
    def run(self):
        clock = pygame.time.Clock()
        
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    if self.state == MENU:
                        self.handle_menu_click(event.pos)
                    elif self.state == PLAYING:
                        self.handle_game_click(event.pos)
                    elif self.state == GAME_OVER:
                        self.state = MENU  # Return to menu after game over
            
            # Update game logic
            if self.state == PLAYING:
                self.update_ai()
                self.check_game_over()
            
            # Draw current state
            if self.state == MENU:
                self.draw_menu()
            elif self.state == PLAYING:
                self.draw_game()
            elif self.state == GAME_OVER:
                self.draw_game_over()
            
            pygame.display.flip()
            clock.tick(60)

if __name__ == "__main__":
    game_ui = GameUI()
    game_ui.run()