#include <iostream>
#include <fstream>
#include <vector>
#include <random>

using namespace std;

// ANSI escape codes: strings used to format console text color/style on supported terminals
#define RESET   "\033[0m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"

class PlayerInfo {
private:
    string username, password;
    int score;
public:
    bool Username (string u_n) {
        ifstream file("players.txt");
        string u, p; int s;
        while (file >> u >> p >> s) {
            if (u == u_n) return false;
        }
        return true;
    }
    void Register () {
        cout << "Enter a valid and unused username: ";
        string un; cin >> un;
        if (Username(un)) {
            username = un;
        } else {
            cout << "Please choose another username as this one already exists!" << endl;
            Register(); return;
        }
        cout << "Enter your password: "; cin >> password;
        string confirm;
        cout << "Re-enter password for confirmation: "; cin >> confirm;
        if (confirm != password) {
            password = " ";
            cout << "Incorrect! Re-enter all your details" << endl;
            Register(); return;
        }
        save_to_file();
        cout << "Registration complete! Your data is saved." << endl;
        cout << endl << "Displaying your info: " << endl;
        score = 0;
        display();
    }
    void display () {
        cout << "Username: " << username << endl;
        cout << "Score: " << score << endl;
    }
    void sign_in () {
        string usnm, pswd;
        cout << "Enter your username: "; cin >> usnm;
        cout << "Enter password: "; cin >> pswd;
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
        ofstream file("players.txt", ios::app);
        file << username << " " << password << " " << score << endl;
        file.close();
    }
    bool verified(string unm, string pwd) {
        ifstream file("players.txt");
        string u, p; int s;
        while (file >> u >> p >> s) {
            if (u == unm && p == pwd) {
                username = u;
                password = p;
                score = s;
                return true;
            }
        }
        cout << "Incorrect username or password!\n";
        return false;
    }
    
    void addScore(int points) {
        score += points; // Update the private score
    }
    //Vector to temporarily store all players; used to rewrite file on update
    void updatePlayerFile() {
        // Read all players from file into memory
        vector<PlayerInfo> allPlayers;
        ifstream inFile("players.txt");
        string u, p;
        int s, r;
        bool found = false;
        while (inFile >> u >> p >> s) {
            PlayerInfo temp;
            temp.username = u;
            temp.password = p;
            temp.score = s;
            // If it's the current user, update the score
            if (u == username) {
                temp.score = score;
                found = true;
            }
            allPlayers.push_back(temp);
        }
        inFile.close();
        // If user not found, add new record
        if (!found) {
            allPlayers.push_back(*this);
        }
        // Write all players back, overwriting the file
        ofstream outFile("players.txt");
        for (auto &player : allPlayers) {
            outFile << player.username << " " << player.password << " " << player.score << endl;
        }
        outFile.close();
    }

};

class SudokuFormation {
private:
    int puzzle [9][9];
    int displayPuzzle[9][9];
    int sub_square [3][3];
    bool userInput[9][9]; // true = entered/modified by the user, false = preset
    PlayerInfo* currentPlayer;

    //Inner Move class to store undo stack entries
    class Move {
    public:
        int row, col, old_value;
        bool old_user_flag;
    };
    Move moveHistory[100];
    int stackTop;
    int playerScore;

    void pushMove(int r, int c, int old_val, bool old_flag) {
        if (stackTop < 99) {
            stackTop++;
            moveHistory[stackTop].row = r;
            moveHistory[stackTop].col = c;
            moveHistory[stackTop].old_value = old_val;
            moveHistory[stackTop].old_user_flag = old_flag;
        } else {
            cout << "Move history full, cannot record new move for undo.\n";
        }
    }
    //Pop last move for undo operation
    Move popMove() {
        if (stackTop >= 0) {
            return moveHistory[stackTop--];
        }
        Move m = {-1, -1, -1, false};
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
                if ((startRow + i != row || startCol + j != col) && displayPuzzle[startRow + i][startCol + j] == val)
                    return false;
            }
        return true;
    }

public:
    SudokuFormation (PlayerInfo* player) {
        currentPlayer = player;
        for (int i=0; i<9 ; i++) {
            for (int j=0; j<9; j++) {
                puzzle[i][j] = 0;
                displayPuzzle[i][j] = 0;
                userInput[i][j] = false;
                if (i < 3 && j < 3) {
                    sub_square[i][j] = 0;
                }
            }
        }
        stackTop = -1;
        playerScore = 0;
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
        for (int i=0; i<9; i++) arr[i] = i+1;
        random_device rd; mt19937 gen(rd());
        for (int i = 8; i > 0; --i) {
            uniform_int_distribution<> dis(0, i);
            int j = dis(gen);
            swap(arr[i], arr[j]);
        }
        int k = 0;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                sub_square[i][j] = arr[k++];
    }
    // Backtracking: fills sudoku grid completely (recursive)
    bool fillGrid() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (puzzle[row][col] == 0) {
                    for (int num = 1; num <= 9; num++) {
                        if (isSafe(row, col, num)) {
                            puzzle[row][col] = num;
                            if (fillGrid())
                                return true;
                            puzzle[row][col] = 0;
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    bool isSafe(int row, int col, int num) {
        for (int x = 0; x < 9; x++) if (puzzle[row][x] == num) return false;
        for (int x = 0; x < 9; x++) if (puzzle[x][col] == num) return false;
        int startRow = row - row % 3, startCol = col - col % 3;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (puzzle[startRow + i][startCol + j] == num) return false;
        return true;
    }

    void hideCells() {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++) {
                displayPuzzle[i][j] = puzzle[i][j];
                userInput[i][j] = false;
            }

        random_device rd; mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 8);
        int cellsToHide = 40;
        while (cellsToHide > 0) {
            int r = dis(gen), c = dis(gen);
            if (displayPuzzle[r][c] != 0) {
                displayPuzzle[r][c] = 0;
                cellsToHide--;
            }
        }
    }

    // Display: user-modified cells are yellow, pre-set are white
    void displayBoard() {
        cout << "\nSudoku Board:\n";
        for(int i=0; i<9; i++) {
            if(i%3 == 0) cout << "-------------------------\n";
            for (int j=0; j<9; j++) {
                if(j%3 == 0) cout << "| ";
                if(displayPuzzle[i][j] == 0) cout << ". ";
                else if (userInput[i][j])
                    cout << YELLOW << displayPuzzle[i][j] << " " << RESET;
                else
                    cout << displayPuzzle[i][j] << " ";
            }
            cout << "|\n";
        }
        cout << "-------------------------\n";
        cout << "Current Score: " << playerScore << endl;
    }

    // Compare: only user-modified cells, green if correct, red if wrong, preset stays white
    void compareWithSolution() {
        cout << "\nComparison (Green: Correct, Red: Wrong, .: Empty, Preset: White)\n";
        int compareScore = 0;
        for(int i=0; i<9; i++) {
            if(i%3 == 0) cout << "-------------------------\n";
            for (int j=0; j<9; j++) {
                if(j%3 == 0) cout << "| ";
                if(displayPuzzle[i][j] == 0) cout << ". ";
                else if (userInput[i][j]) {
                    if(displayPuzzle[i][j] == puzzle[i][j]) {
                        cout << GREEN << displayPuzzle[i][j] << " " << RESET;
                        compareScore += 3;
                    }
                    else {
                        cout << RED << displayPuzzle[i][j] << " " << RESET;
                        compareScore -= 3;
                    }
                }
                else {
                    cout << displayPuzzle[i][j] << " "; // preset is white/normal
                }
            }
            cout << "|\n";
        }
        cout << "-------------------------\n";
        cout << "Score from comparison: " << compareScore << endl;
        playerScore += compareScore;
        cout << "Current Score: " << playerScore << endl;
    }

    void play() {
        while (true) {
            displayBoard();
            int r, c, val;
            cout << "\nEnter (row col value), 0 to undo, -1 to exit, or -2 to compare: "<<endl;
            cout << "Row: ";
            cin >> r;

            if (r == -1) break;

            if (r == -2) {
                compareWithSolution();
                continue;
            }

            if (r == 0) {
                if (stackTop >= 0) {
                    Move lastMove = popMove();
                    displayPuzzle[lastMove.row][lastMove.col] = lastMove.old_value;
                    userInput[lastMove.row][lastMove.col] = lastMove.old_user_flag;
                    playerScore -= 5;
                } else {
                    cout << "No moves to undo! Move history is empty." << endl;
                }
                continue;
            }
            
            cout << "Column: ";
            cin >> c;
            cout << "Value: ";
            cin >> val;
            int row_index = r - 1;
            int col_index = c - 1;

            if (r >= 1 && r <= 9 && c >= 1 && c <= 9 && val >= 1 && val <= 9) {
                if (!userInput[row_index][col_index] && displayPuzzle[row_index][col_index] != 0) {
                    cout << "Cannot modify original puzzle value! Try again.\n";
                    continue;
                }
                int old_val = displayPuzzle[row_index][col_index];
                bool old_flag = userInput[row_index][col_index];
                displayPuzzle[row_index][col_index] = val;

                if (isValid(row_index, col_index, val)) {
                    playerScore += 1;
                    userInput[row_index][col_index] = true;
                    cout << YELLOW << "Move accepted!\n" << RESET;
                    pushMove(row_index, col_index, old_val, old_flag);
                } else {
                    playerScore -= 3;
                    displayPuzzle[row_index][col_index] = old_val;
                    cout << RED << "Wrong move! This placement violates Sudoku rules.\n" << RESET;
                }
            } else {
                cout << "Invalid input! Row, column, or value is out of range (1-9).\n";
            }
        }
    }
    
    void finalizeScore() {
        if (currentPlayer) {
            currentPlayer->addScore(playerScore);
            currentPlayer->updatePlayerFile();
        }
    }

};

int main () {
    string YESorNO;
    cout << "Hey! Are you an existing player? [YES/NO]" << endl;
    cin >> YESorNO;
    if (YESorNO == "YES" || YESorNO == "Y"|| YESorNO == "yes"|| YESorNO == "y") {
        PlayerInfo p;
        p.sign_in();

        SudokuFormation* p1 = new SudokuFormation(&p);
        p1->completePuzzle();
        cout << "Let's begin!!!" << endl;
        p1->play();
        p1->finalizeScore();
        cout << endl;
        p.display();
        delete p1;
        
        cout << "Do you wish to play again? [YES/NO]" << endl;
        cin >> YESorNO;
        
        if (YESorNO == "YES" || YESorNO == "Y"|| YESorNO == "yes"|| YESorNO == "y") {
            SudokuFormation p1(&p);
            p1.completePuzzle();
            cout << "Let's begin!!!" << endl;
            p1.play();
            p1.finalizeScore();
            cout << endl;
            p.display();
            
            cout << "Thank you for playing!" << endl;
        } else if (YESorNO == "NO" || YESorNO == "N"|| YESorNO == "no"|| YESorNO == "n") {
            cout << "Thank you for playing! See ya later!!!" << endl;
        } else {
            cout << "Incorrect input";
        }
        
    } else if (YESorNO == "NO" || YESorNO == "N"|| YESorNO == "no"|| YESorNO == "n") {
        PlayerInfo new_p;
        new_p.Register();

        SudokuFormation* p1 = new SudokuFormation(&new_p);
        p1->completePuzzle();
        cout << "Let's begin!!!" << endl;
        p1->play();
        p1->finalizeScore();
        cout << endl;
        new_p.display();
        delete p1;
        
        cout << "Do you wish to play again? [YES/NO]" << endl;
        cin >> YESorNO;
        if (YESorNO == "YES" || YESorNO == "Y"|| YESorNO == "yes"|| YESorNO == "y") {
            SudokuFormation p1(&new_p);
            p1.completePuzzle();
            cout << "Let's begin!!!" << endl;
            p1.play();
            p1.finalizeScore();
            cout << endl;
            new_p.display();
            
            cout << "Thank you for playing!" << endl;
        } else if (YESorNO == "NO" || YESorNO == "N"|| YESorNO == "no"|| YESorNO == "n") {
            cout << "Thank you for playing! See ya later!!!" << endl;
        } else {
            cout << "Incorrect input";
        }
    } else {
        cout << "Invalid input!" << endl;
        main();
    }
    
    return 0;
}
