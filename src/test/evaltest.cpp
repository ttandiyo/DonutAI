#include "../AIShell.cpp"
#include "../AIShell.h"
#include "../LineCounter.h"
#include "../Move.cpp"
#include "../Move.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

bool isFirstPlayer = false;

int main() {
    while (1) {
        AIShell *shell = NULL;

        bool gravity = false;

        int colCount = 3;
        int rowCount = 3;
        int lastMoveCol = 1;
        int lastMoveRow = 1;

        int deadline = 5000;
        int k = 3;

        int **gameState = NULL;
        gameState = new int *[colCount];
        for (int i = 0; i < colCount; i++) {
            gameState[i] = new int[rowCount];
        }

        int countMoves = 0;
        for (int col = 0; col < colCount; col++) {
            for (int row = 0; row < rowCount; row++) {
                cin >> gameState[col][row];
                if (gameState[col][row] != AIShell::NO_PIECE) {
                    countMoves += gameState[col][row];
                }
            }
        }

        if (countMoves % 2 == 0) {
            isFirstPlayer = true;
        }

        for (int y = rowCount - 1; y >= 0; y--) {
            cout << "[\t";
            for (int x = 0; x < colCount; x++) {
                if (gameState[x][y] == AIShell::MAX_PIECE) {
                    cout << "[P]\t";
                } else if (gameState[x][y] == AIShell::MIN_PIECE) {
                    cout << "[O]\t";
                } else {
                    cout << gameState[x][y] << "\t";
                }
            }
            cout << "]" << endl;
        }

        Move m(lastMoveCol, lastMoveRow);

        shell = new AIShell(colCount, rowCount, gravity, gameState, m);
        shell->deadline = deadline;
        shell->k = k;
        std::vector<std::pair<int, int>> moves{{1, 1}, {1, 2}};
        // cout << boolalpha;
        // cout << moves.empty() << endl;
        int result = shell->staticEval(moves);
        cout << result << endl;
        delete shell;
    }
}
