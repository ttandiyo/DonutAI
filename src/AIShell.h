#ifndef AISHELL_H
#define AISHELL_H

#include "Move.h"
#include <chrono>
#include <vector>

namespace {
typedef std::pair<int, int> Coord;
typedef std::vector<Coord> MovesList;
}

// A new AIShell will be created for every move request.
class AIShell {
  public:
    // these represent the values for each piece type.
    static const int PLAYER_PIECE = 1;
    static const int OPPONENT_PIECE = -1;
    static const int NO_PIECE = 0;

  private:
    // Do not alter the values of numRows or numcols.
    // they are used for deallocating the gameState variable.
    const bool gravityOn; // this will be true if gravity is turned on. It will
                          // be false if gravity is turned off.
    const int numCols;    // the total number of columns in the game state.
    const int numRows;    // the total number of rows in the game state.
    const int k;
    int **gameState;     // a pointer to a two-dimensional array representing
                         // the game state.
    const Move lastMove; // this is the move made last by your opponent. If your
                         // opponent has not made a move yet (you move first)
                         // then this move will hold the value (-1, -1) instead.
    const int deadline;
    MovesList moves;
    std::chrono::milliseconds startTime;
    std::chrono::milliseconds stopTime;
    bool outOfTime = false;

    bool timeLeft() const;
    void checkTime();
    const Coord dropPiece(const int col) const;
    bool colHasSpace(const int col) const;
    bool notInMoves(const Coord move) const;
    bool colNotEmpty(const int col) const;
    void findMoves();
    int staticEval() const;
    const Move runIDS();
    const std::pair<Coord, int> startMiniMax(const int depth);
    const std::pair<MovesList, int> minPlayer(const MovesList moves,
                                              const int depth, int a, int b);
    const std::pair<MovesList, int> maxPlayer(const MovesList moves,
                                              const int depth, int a, int b);

  public:
    // int deadline; // this is how many milliseconds the AI has to make move.
    // int k;        // k is the number of pieces a player must get in a
    // row/column/diagonal to win the game. IE in connect 4, this
    // variable would be 4

    AIShell(bool gravityOn, int numCols, int numRows, int k, int **gameState,
            Move lastMove, int deadline);
    ~AIShell();
    Move makeMove();
};

#endif // AISHELL_H
