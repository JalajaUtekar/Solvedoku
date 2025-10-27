#include <iostream>
#include <fstream>
#include <vector>
#include <random>

using namespace std;

class SudokuFormation {
private:
    int puzzle [9][9];
    int displayPuzzle[9][9];
    int sub_square [3][3];

    class Move {
    public:
        int row, col, old_value;
    };
    Move moveHistory[100];
    int stackTop;
    int playerScore; // Added for in-game scoring

    void pushMove(int r, int c, int old_val) {
        if (stackTop < 99) {
            stackTop++;
            moveHistory[stackTop].row = r;
            moveHistory[stackTop].col = c;
            moveHistory[stackTop].old_value = old_val;
        } else {
            cout << "Move history full, cannot record new move for undo.\n";
        }
    }

    Move popMove() {
        if (stackTop >= 0) {
            return moveHistory[stackTop--];
        }
        Move m = {-1, -1, -1};
        return m;
    }

    bool isValid(int row, int col, int val) {
        for (int i = 0; i < 9; i++) {
            if (i != col && displayPuzzle[row][i] == val) return false;
            if (i != row && displayPuzzle[i][col] == val) return false;
        }

        int startRow = (row / 3) * 3;
        int startCol = (col / 3) * 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                if (startRow + i != row || startCol + j != col) {
                    if (displayPuzzle[startRow + i][startCol + j] == val)
                        return false;
                }
            }

        return true;
    }

public:
    SudokuFormation () {
        for (int i=0; i<9 ; i++) {
            for (int j=0; j<9; j++) {
                puzzle[i][j] = 0;
                displayPuzzle[i][j] = 0;
                if (i < 3 && j < 3) {
                    sub_square[i][j] = 0;
                }
            }
        }
        stackTop = -1;
        playerScore = 0; // Initialize score to zero
    }

    void completePuzzle () {
        setDiagonalSquares();
        fillGrid();
        hideCells();
    }

    void setDiagonalSquares () {
        setDiagonalSubSquare();
        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                puzzle [i][j] = sub_square[i][j];
                sub_square[i][j] = 0;
            }
        }
        setDiagonalSubSquare();
        for (int i=3; i<6; i++) {
            for (int j=3; j<6; j++) {
                puzzle [i][j] = sub_square[i-3][j-3];
                sub_square[i-3][j-3] = 0;
            }
        }
        setDiagonalSubSquare();
        for (int i=6; i<9; i++) {
            for (int j=6; j<9; j++) {
                puzzle [i][j] = sub_square[i-6][j-6];
                sub_square[i-6][j-6] = 0;
            }
        }
    }

    void setDiagonalSubSquare() {
        int arr[9];
        for (int i=0; i<9; i++) {
            arr[i] = i+1;
        }

        // Shuffle the digits (Fisher-Yates)
        random_device rd;
        mt19937 gen(rd());
        for (int i = 8; i > 0; --i) {
            uniform_int_distribution<> dis(0, i);
            int j = dis(gen);
            swap(arr[i], arr[j]);
        }

        int k = 0;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sub_square[i][j] = arr[k++];
            }
        }
    }

    bool fillGrid() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (puzzle[row][col] == 0) {
                    for (int num = 1; num <= 9; num++) {
                        if (isSafe(row, col, num)) {
                            puzzle[row][col] = num;
                            if (fillGrid())
                                return true;
                            puzzle[row][col] = 0; // Backtrack!
                        }
                    }
                    return false; // If no number fits, trigger backtracking
                }
            }
        }
        return true; // If no empty cell found, grid is filled!
    }

    bool isSafe(int row, int col, int num) {
        // Row check
        for (int x = 0; x < 9; x++) {
            if (puzzle[row][x] == num) {
                return false;
            }
        }

        // Column check
        for (int x = 0; x < 9; x++) {
            if (puzzle[x][col] == num) {
                return false;
            }
        }

        // 3x3 box check
        int startRow = row - row % 3;
        int startCol = col - col % 3;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (puzzle[startRow + i][startCol + j] == num) {
                    return false;
                }
            }
        }
        return true;
    }

    void hideCells() {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                displayPuzzle[i][j] = puzzle[i][j];

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 8);

        int cellsToHide = 40;
        while (cellsToHide > 0) {
            int r = dis(gen);
            int c = dis(gen);
            if (displayPuzzle[r][c] != 0) {
                displayPuzzle[r][c] = 0;
                cellsToHide--;
            }
        }
    }

    void displayBoard() {
        cout << "\nSudoku Board:\n";
        cout << "\n  1 2 3   4 5 6   7 8 9\n";
        for(int i=0;i<9;i++) {
            if(i%3 == 0) {
                cout << "-------------------------\n";
            }

            cout << i + 1 << " "; 

            for (int j=0;j<9;j++) {
                if(j%3 == 0) {
                    cout << "| ";
                }
                if(displayPuzzle[i][j] == 0) {
                    cout << ". ";
                }
                else {
                    cout << displayPuzzle[i][j] << " ";
                }
            }
            cout << "|\n";
        }
        cout << "-------------------------\n";
        cout << "Current Score: " << playerScore << endl; // Display score
    }

    void play() {
        while (true) {
            displayBoard();
            int r, c, val;
            cout << "\nEnter (row col value), 0 to undo, or -1 to exit: ";
            cin >> r;

            if (r == -1) break;

            // UNDO Logic
            if (r == 0) {
                if (stackTop >= 0) {
                    Move lastMove = popMove();
                    displayPuzzle[lastMove.row][lastMove.col] = lastMove.old_value;
                    playerScore -= 5; // Deduct points for undo [web:1]
                } else {
                    cout << "No moves to undo! Move history is empty." << endl;
                }
            continue;
            }
            cin >> c >> val;
            int row_index = r - 1;
            int col_index = c - 1;

            if (r >= 1 && r <= 9 && c >= 1 && c <= 9 && val >= 1 && val <= 9) {
                int old_val = displayPuzzle[row_index][col_index];
                displayPuzzle[row_index][col_index] = val;

                if (isValid(row_index, col_index, val)) {
                    playerScore += 1;
                    cout << "Move accepted!\n";
                    if (old_val != 0 || val != 0) {
                        pushMove(row_index, col_index, old_val);
                    }
                } else {
                playerScore -= 3;
                displayPuzzle[row_index][col_index] = old_val;
                cout << "Wrong move! This placement violates Sudoku rules.\n";
                }
            } else {
                cout << "Invalid input! Row, column, or value is out of range (1-9).\n";
            }
        }
    }

};

class PlayerInfo {
private:
    string username, password;
    int score, rank;
public:
    PlayerInfo () {
        score = 0;
        rank = 0;
    }

    bool Username (string u_n) {
        ifstream file("players.txt");
        string u, p;
        int s, r;
        while (file >> u >> p >> s >> r) {
            if (u == u_n) return false;  // username already exists
        }
        return true;
    }

    void Register () {
        cout << "Enter a valid and unused username: ";
        string un;
        cin >> un;
        if (Username(un)) {
            username = un;
        } else {
            cout << "Please choose another username as this one already exists!" << endl;
            Register();
        }
        cout << "Enter your password: ";
        cin >> password;
        string confirm;
        cout << "Re-enter password for confirmation: ";
        cin >> confirm;
        if (confirm != password) {
            password = " ";
            cout << "Incorrect! Re-enter all your details" << endl;
            Register();
        }
        save_to_file();
        cout << "Registration complete! Your data is saved." << endl;

        cout << endl << "Displaying your info: " << endl;
        display();
    }

    void display () {
        cout << "Username: " << username << endl;
        cout << "Rank: " << rank << endl;
        cout << "Score: " << score << endl;
    }

    void sign_in () {
        string usnm, pswd;
        cout << "Enter your username: ";
        cin >> usnm;
        cout << "Enter password: ";
        cin >> pswd;
        if (verified(usnm, pswd)) {
            username = usnm;
            password = pswd;
            cout << endl << "Displaying your info: " << endl;
            display();
            cout << endl << "Welcome back to the Sudoku game!" << endl;
        } else {
            string yn;
            cout << endl << "Do you really have an account? (YES/NO)" << endl;
            cin >> yn;
            if (yn == "YES") {
                sign_in();
            } else if (yn == "NO") {
                Register();
            } else {
                cout << "Bro answer in caps please!" << endl;
            }
        }
    }

    void save_to_file() {
        ofstream file("players.txt", ios::app); // append mode
        file << username << " " << password << " " << score << " " << rank << endl;
        file.close();
    }

    bool verified(string unm, string pwd) {
        ifstream file("players.txt");
        string u, p;
        int s, r;
        while (file >> u >> p >> s >> r) {
            if (u == unm && p == pwd) {
                username = u;
                password = p;
                score = s;
                rank = r;
                return true;
            }
        }
        cout << "Incorrect username or password!\n";
        return false;
    }
};

int main () {
    string YESorNO;
    cout << "Hey! Are you an existing player? [YES/NO]" << endl;
    cin >> YESorNO;
    if (YESorNO == "YES" || YESorNO == "Y"|| YESorNO == "yes"|| YESorNO == "y") {
        PlayerInfo p;
        p.sign_in();

    } else if (YESorNO == "NO" || YESorNO == "N"|| YESorNO == "no"|| YESorNO == "n") {
        PlayerInfo new_p;
        new_p.Register();
    } else {
        cout << "Invalid input!" << endl;
        main();
    }

    SudokuFormation p1;
    p1.completePuzzle();
    p1.play();

    return 0;
}
