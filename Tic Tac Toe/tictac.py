import tkinter as tk
from tkinter import messagebox

AI_player = 'X'
human_player = 'O'
board = [['_', '_', '_'] for _ in range(3)]
marked = {}

def have_a_move(board):
    for i in range(3):
        for j in range(3):
            if board[i][j] == '_':
                return True
    return False

def evaluate(board):
    for row in range(3):
        if board[row][0] == board[row][1] == board[row][2] and board[row][0] != '_':
            return 10 if board[row][0] == AI_player else -10

    for col in range(3):
        if board[0][col] == board[1][col] == board[2][col] and board[0][col] != '_':
            return 10 if board[0][col] == AI_player else -10

    if board[0][0] == board[1][1] == board[2][2] and board[0][0] != '_':
        return 10 if board[0][0] == AI_player else -10

    if board[0][2] == board[1][1] == board[2][0] and board[0][2] != '_':
        return 10 if board[0][2] == AI_player else -10

    return 0

def minimax(board, isMax):
    score = evaluate(board)
    if score == 10:
        return score
    if score == -10:
        return score
    if not have_a_move(board):
        return 0

    if isMax:
        best = -1000
        for i in range(3):
            for j in range(3):
                if board[i][j] == '_':
                    board[i][j] = AI_player
                    best = max(best, minimax(board, not isMax))
                    board[i][j] = '_'
        return best
    else:
        best = 1000
        for i in range(3):
            for j in range(3):
                if board[i][j] == '_':
                    board[i][j] = human_player
                    best = min(best, minimax(board, not isMax))
                    board[i][j] = '_'
        return best
    
def find_best_move(board):
    bestVal = -1000
    row, col = -1, -1

    for i in range(3):
        for j in range(3):
            if board[i][j] == '_':
                board[i][j] = AI_player
                moveVal = minimax(board, False)
                board[i][j] = '_'
                if moveVal > bestVal:
                    row, col = i, j
                    bestVal = moveVal
    return row, col

def print_board(board):
    for i in range(3):
        for j in range(3):
            print(f"| {'X' if board[i][j] == 'X' else 'O' if board[i][j] == 'O' else board[i][j]} ", end='')
        print('|')

def on_button_click(row, col):
    global board, marked
    if board[row][col] == '_' and not marked.get((row, col)):
        board[row][col] = 'O'
        marked[(row, col)] = True
        update_gui_board(row, col, 'green')
        if evaluate(board) == -10:
            messagebox.showinfo("Game Over", "You Win!")
            window.destroy()
        elif have_a_move(board):
            best_move = find_best_move(board)
            ai_row, ai_col = best_move
            board[ai_row][ai_col] = 'X'
            marked[(ai_row, ai_col)] = True
            update_gui_board(ai_row, ai_col, 'blue')
            if evaluate(board) == 10:
                messagebox.showinfo("Game Over", "AI Player Wins!")
                window.destroy()
        else:
            messagebox.showinfo("Game Over", "Tied!")
            window.destroy()

def update_gui_board(row=None, col=None, color=None):
    for i in range(3):
        for j in range(3):
            button_texts[i][j].set(board[i][j])
            if row is not None and col is not None and i == row and j == col and color:
                buttons[i][j].config(bg=color)


window = tk.Tk()
window.title("Tic-Tac-Toe")
window.geometry("300x300")

button_texts = [[tk.StringVar() for _ in range(3)] for _ in range(3)]
buttons = [[None for _ in range(3)] for _ in range(3)]
for i in range(3):
    for j in range(3):
        buttons[i][j] = tk.Button(window, textvariable=button_texts[i][j],
                                  width=8, height=5,
                                  command=lambda row=i, col=j: on_button_click(row, col))
        buttons[i][j].grid(row=i, column=j, sticky="nsew")


update_gui_board()
window.mainloop()
