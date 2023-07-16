#include "AIShell.h"
#include "LineCounter.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

// #define DEBUG

using namespace std;
using namespace std::chrono;

namespace {
static const bool DEBUG = false;
static const std::chrono::milliseconds TIME_MARGIN{0};

std::random_device rd;
std::mt19937 rng(rd()); // rng to return random move when no best move found

static const int MAXWIN = 1000;
static const int MINWIN = -1000;

// Store the last best move chain statically since AIShell is destroyed each
// turn.
// Kludgy- it would be better if the history could be stored inside AIShell.
MovesList lastMoves = {};
} // namespace
inline const milliseconds currentTime() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

inline LineCounter::LineCounter(int k) : k{k} {}

inline int LineCounter::winsInLine(int lineLength) const {
  return (lineLength >= k) ? (lineLength - k + 1) : 0;
}

inline int LineCounter::possibleWinsInLine(int lineLength) const {
  return winsInLine(lineLength);
}

inline void LineCounter::pushContiguous() {
  minWins += winsInLine(minContiguousStreak);
  maxWins += winsInLine(maxContiguousStreak);
  minContiguousStreak = 0;
  maxContiguousStreak = 0;
}

inline void LineCounter::pushMax() {
  maxWins += winsInLine(maxContiguousStreak);
  maxPossibleWins += possibleWinsInLine(maxOpenStreak);
  maxOpenStreak = 0;
  maxContiguousStreak = 0;
}

inline void LineCounter::pushMin() {
  minWins += winsInLine(minContiguousStreak);
  minPossibleWins += possibleWinsInLine(minOpenStreak);
  minOpenStreak = 0;
  minContiguousStreak = 0;
}

inline void LineCounter::pushLine() {
  minWins += winsInLine(minContiguousStreak);
  maxWins += winsInLine(maxContiguousStreak);
  minPossibleWins += possibleWinsInLine(minOpenStreak);
  maxPossibleWins += possibleWinsInLine(maxOpenStreak);
  minOpenStreak = 0;
  maxOpenStreak = 0;
  minContiguousStreak = 0;
  maxContiguousStreak = 0;
}

inline void LineCounter::addPiece(int piece) {
  switch (piece) {
  case AIShell::NO_PIECE:
    minOpenStreak++;
    maxOpenStreak++;
    pushContiguous();
    break;
  case AIShell::OPPONENT_PIECE:
    minOpenStreak++;
    minContiguousStreak++;
    pushMax();
    break;
  case AIShell::PLAYER_PIECE:
    maxOpenStreak++;
    maxContiguousStreak++;
    pushMin();
    break;
  default:
    break;
  }
}

AIShell::AIShell(bool gravityOn, int numCols, int numRows, int k,
                 int **gameState, Move lastMove, int deadline)
    : gravityOn{gravityOn}, numCols{numCols}, numRows{numRows}, k{k},
      gameState{gameState}, lastMove{lastMove}, deadline{deadline} {}

AIShell::~AIShell() {
  // delete the gameState variable.
  for (int i = 0; i < numCols; i++) {
    delete[] gameState[i];
  }
  delete[] gameState;
}

inline bool AIShell::timeLeft() const { return currentTime() < stopTime; }

inline void AIShell::checkTime() {
  if (!timeLeft()) {
    outOfTime = true;
  }
}

inline const Coord AIShell::dropPiece(const int col) const {
  for (int row = 0; row < numRows; row++) {
    if (gameState[col][row] == NO_PIECE) {
      return {col, row};
    }
  }
  if (DEBUG) {
    cout << "dropPiece: Invalid column specified: index " << col << endl;
  }
  return {-1, -1}; // Make sure we never get here
}

inline bool AIShell::colHasSpace(const int col) const {

  if (col < 0 or col > (numCols - 1)) {
    if (DEBUG) {
      cout << "colHasSpace: Invalid column specified: index " << col << endl;
    }
    return false;
  }
  return gameState[col][numRows - 1] == NO_PIECE;
}

inline bool AIShell::colNotEmpty(const int col) const {
  return gameState[col][0] != NO_PIECE;
}

inline bool AIShell::notInMoves(const Coord move) const {
  return find(moves.begin(), moves.end(), move) == moves.end();
}

inline void AIShell::findMoves() {
  moves = {};

  for (auto move : lastMoves) {
    if (gravityOn) { // Prioritize columns from lastMoves
      if (colHasSpace(move.first) and notInMoves(dropPiece(move.first))) {
        moves.push_back(dropPiece(move.first));
      }
    } else if (!gravityOn) { // Prioritize moves from lastMoves
      if (gameState[move.first][move.second] == NO_PIECE and notInMoves(move)) {
        moves.push_back(move);
      }
    }
  }

  // lastMoves.clear();

  if (gravityOn) { // Prioritize moves in non-empty columns & their neighbors
    for (int col = 0; col < numCols; col++) {
      if (colNotEmpty(col)) {
        for (int col2 = max(0, col - 1); col2 <= min(col + 1, numCols - 1);
             col2++) {
          if (colHasSpace(col2) and notInMoves(dropPiece(col2))) {
            moves.push_back(dropPiece(col2));
          }
        }
      }
    }
  } else if (!gravityOn) { // Prioritize moves around existing pieces
    for (int col = 0; col < numCols; col++) {
      for (int row = 0; row < numRows; row++) {
        if (gameState[col][row] != NO_PIECE) {
          for (int x = max(0, col - 1); x <= min(col + 1, numCols - 1); x++) {
            if (gameState[x][row] == NO_PIECE and notInMoves({x, row})) {
              moves.push_back({x, row});
            }
          }
          for (int y = max(0, row - 1); y <= min(row + 1, numRows - 1); y++) {
            if (gameState[col][y] == NO_PIECE and notInMoves({col, y})) {
              moves.push_back({col, y});
            }
          }
          for (int x = max(0, col - 1), y = max(0, row - 1);
               x <= min(col + 1, numCols - 1) and
               y <= min(row + 1, numRows - 1);
               x++, y++) {
            if (gameState[x][y] == NO_PIECE and notInMoves({x, y})) {
              moves.push_back({x, y});
            }
          }
          for (int x = max(0, col - 1), y = min(row + 1, numRows - 1);
               x <= min(col + 1, numCols - 1) and y >= max(0, row - 1);
               x++, y--) {
            if (gameState[x][y] == NO_PIECE and notInMoves({x, y})) {
              moves.push_back({x, y});
            }
          }
        }
      }
    }
  }

  if (gravityOn) { // Add the rest of the non-filled columns
    for (int col = 0; col < numCols; col++) {
      if (colHasSpace(col) and notInMoves(dropPiece(col))) {
        moves.push_back(dropPiece(col));
      }
    }
  } else if (!gravityOn) { // Add the rest of the pieces
    for (int col = 0; col < numCols; col++) {
      for (int row = 0; row < numRows; row++) {
        if (gameState[col][row] == NO_PIECE and notInMoves({col, row})) {
          moves.push_back({col, row});
        }
      }
    }
  }
}

inline int AIShell::staticEval() const {
  // TODO: Count k-1 in a row placed pieces with 1 empty piece
  // TODO: Count k-2 in a row placed pieces with 2 empty pieces

  // Boundaries are defined inclusively.

  // This box of k around is valid for calculating a winner, but it doesn't
  // provide valid relative overall utility scores for comparing states.
  // It is faster than calculating the whole board; use it if the
  // heuristic is switched out for something else.

  // const int left = max(lastMove.col - (k - 1), 0),
  //           bottom = max(lastMove.row - (k - 1), 0),
  //           right = min(lastMove.col + (k - 1), numCols - 1),
  //           top = min(lastMove.row + (k - 1), numRows - 1);

  const int left = 0, bottom = 0, right = numCols - 1, top = numRows - 1;

  LineCounter verticalCounter = LineCounter(k);
  LineCounter horizCounter = LineCounter(k);
  LineCounter downRightCounter = LineCounter(k);
  LineCounter downLeftCounter = LineCounter(k);

  // vertical
  for (int x = left; x <= right; x++) {
    for (int y = bottom; y <= top; y++) {
      verticalCounter.addPiece(gameState[x][y]);
    }
    verticalCounter.pushLine();
  }

  // horizontal
  for (int y = bottom; y <= top; y++) {
    for (int x = left; x <= right; x++) {
      horizCounter.addPiece(gameState[x][y]);
    }
    horizCounter.pushLine();
  }

  // downright diagonal
  for (int row = bottom + (k - 1); row <= top; row++) {
    for (int x = left, y = row; y >= bottom and x <= right; x++, y--) {
      downRightCounter.addPiece(gameState[x][y]);
    }
    downRightCounter.pushLine();
  }
  for (int col = left + 1; col <= right - (k - 1); col++) {
    for (int y = top, x = col; y >= bottom and x <= right; x++, y--) {
      downRightCounter.addPiece(gameState[x][y]);
    }
    downRightCounter.pushLine();
  }

  // downleft diagonal
  for (int col = left + (k - 1); col <= right; col++) {
    for (int x = col, y = top; x >= left and y >= bottom; x--, y--) {
      downLeftCounter.addPiece(gameState[x][y]);
    }
    downLeftCounter.pushLine();
  }
  for (int row = top - 1; row >= bottom; row--) {
    for (int x = right, y = row; x >= left and y >= bottom; x--, y--) {
      downLeftCounter.addPiece(gameState[x][y]);
    }
    downLeftCounter.pushLine();
  }

  bool minWinner = verticalCounter.minWins or horizCounter.minWins or
                   downLeftCounter.minWins or downRightCounter.minWins;
  bool maxWinner = verticalCounter.maxWins or horizCounter.maxWins or
                   downLeftCounter.maxWins or downRightCounter.maxWins;

  if (minWinner) {
    return MINWIN;
  } else if (maxWinner) {
    return MAXWIN;
  } else {
    int maxScore =
        verticalCounter.maxPossibleWins + horizCounter.maxPossibleWins +
        downLeftCounter.maxPossibleWins + downRightCounter.maxPossibleWins;
    int minScore =
        verticalCounter.minPossibleWins + horizCounter.minPossibleWins +
        downLeftCounter.minPossibleWins + downRightCounter.minPossibleWins;
    return maxScore - minScore;
  }
}

inline const pair<MovesList, int>
AIShell::minPlayer(const MovesList moves, const int depth, int a, int b) {
  const int evalScore = staticEval();
  if ((depth == 0) or (evalScore == MINWIN) or (evalScore == MAXWIN) or
      moves.empty()) {
    return {{}, evalScore};
  }

  MovesList moveHistory = {};
  int worstScore = MAXWIN;

  for (int i = 0; i < moves.size(); i++) {
    Coord move = moves[i];
    gameState[move.first][move.second] = OPPONENT_PIECE;
    MovesList newMoves = {};
    if (gravityOn and colHasSpace(move.first)) {
      newMoves.push_back(dropPiece(move.first));
    }
    for (int j = 0; j < i; j++) {
      newMoves.push_back(moves[j]);
    }
    for (int j = i + 1; j < moves.size(); j++) {
      newMoves.push_back(moves[j]);
    }
    MovesList childMoveHistory;
    int stateScore;
    tie(childMoveHistory, stateScore) = maxPlayer(newMoves, depth - 1, a, b);
    gameState[move.first][move.second] = NO_PIECE;
    if (stateScore < worstScore) {
      worstScore = stateScore;
      moveHistory.clear();
      moveHistory.push_back(move);
      // if (!childMoveHistory.empty()) {
      for (auto item : childMoveHistory) {
        moveHistory.push_back(item);
      }
      // }
    }
    b = min(stateScore, b);

    checkTime();

    if (a >= b or outOfTime) {
      break;
    }
  }

  return {moveHistory, worstScore};
}

inline const pair<MovesList, int>
AIShell::maxPlayer(const MovesList moves, const int depth, int a, int b) {
  const int evalScore = staticEval();
  if ((depth == 0) or (evalScore == MINWIN) or (evalScore == MAXWIN) or
      moves.empty()) {
    return {{}, evalScore};
  }

  MovesList moveHistory = {};
  int bestScore = MINWIN;

  for (int i = 0; i < moves.size(); i++) {
    Coord move = moves[i];
    gameState[move.first][move.second] = PLAYER_PIECE;
    MovesList newMoves = {};
    if (gravityOn and colHasSpace(move.first)) {
      newMoves.push_back(dropPiece(move.first));
    }
    for (int j = 0; j < i; j++) {
      newMoves.push_back(moves[j]);
    }
    for (int j = i + 1; j < moves.size(); j++) {
      newMoves.push_back(moves[j]);
    }
    MovesList childMoveHistory;
    int stateScore;
    tie(childMoveHistory, stateScore) = minPlayer(newMoves, depth - 1, a, b);
    gameState[move.first][move.second] = NO_PIECE;
    if (stateScore > bestScore) {
      bestScore = stateScore;
      moveHistory.clear();
      moveHistory.push_back(move);
      // if (!childMoveHistory.empty()) {
      for (auto item : childMoveHistory) {
        moveHistory.push_back(item);
      }
      // }
    }
    a = max(bestScore, a);

    checkTime();

    if (a >= b or outOfTime) {
      break;
    }
  }

  return {moveHistory, bestScore};
}

const pair<Coord, int> AIShell::startMiniMax(const int depth) {
  MovesList moveHistory;
  uniform_int_distribution<int> uni(0, moves.size() - 1);
  Coord bestMove = moves[uni(rng)];
  int a = MINWIN;         // lost = true
  int b = MAXWIN;         // won = true
  int bestScore = MINWIN; // lost = true

  vector<vector<int>> scoreBoard;
  if (DEBUG) {
    scoreBoard.resize(numCols, vector<int>(numRows, 0));
  }

  for (int i = 0; i < moves.size(); i++) {
    Coord move = moves[i];
    MovesList newMoves = {};
    gameState[move.first][move.second] = PLAYER_PIECE;
    if (gravityOn and colHasSpace(move.first)) {
      newMoves.push_back(dropPiece(move.first));
    }
    for (int j = 0; j < i; j++) {
      newMoves.push_back(moves[j]);
    }
    for (int j = i + 1; j < moves.size(); j++) {
      newMoves.push_back(moves[j]);
    }
    MovesList childMoveHistory;
    int stateScore;
    tie(childMoveHistory, stateScore) = minPlayer(newMoves, depth - 1, a, b);
    gameState[move.first][move.second] = NO_PIECE;
    if (stateScore > bestScore) {
      bestScore = stateScore;
      moveHistory = childMoveHistory;
      bestMove = move;
    }
    a = max(bestScore, a);

    if (DEBUG) {
      scoreBoard[move.first][move.second] = stateScore;
    }

    checkTime();

    if (a >= b or outOfTime) {
      break;
    }
  }

  if (!outOfTime) {
    lastMoves.clear();
    lastMoves.push_back(bestMove);
    if (!moveHistory.empty()) {
      for (auto item : moveHistory) {
        lastMoves.push_back(item);
      }
    }
  }

  if (DEBUG and !outOfTime) {
    // Print score board
    for (int y = numRows - 1; y >= 0; y--) {
      cout << "[\t";
      for (int x = 0; x < numCols; x++) {
        if (gameState[x][y] == PLAYER_PIECE) {
          cout << "[P]\t";
        } else if (gameState[x][y] == OPPONENT_PIECE) {
          cout << "[E]\t";
        } else if (x == bestMove.first and y == bestMove.second) {
          cout << "(";
          if (scoreBoard[x][y] == MAXWIN) {
            cout << "W!";
          } else if (scoreBoard[x][y] == MINWIN) {
            cout << "L!";
          } else {
            cout << scoreBoard[x][y];
          }
          cout << ")\t";
        } else {
          if (scoreBoard[x][y] == MAXWIN) {
            cout << "W!\t";
          } else if (scoreBoard[x][y] == MINWIN) {
            cout << "L!\t";
          } else if (!notInMoves({x, y})) {
            cout << scoreBoard[x][y] << "\t";
          } else {
            cout << "-\t";
          }
        }
      }
      cout << "]" << endl;
    }
    // Print expected path
    cout << "Expected path: ";
    for (int i = 0; i < lastMoves.size(); i++) {
      if (i != 0) {
        cout << "->";
      }
      cout << "(" << lastMoves[i].first << ", " << lastMoves[i].second << ")";
    }
    cout << endl << endl;
  }

  return {bestMove, bestScore};
}

const Move AIShell::runIDS() {
  findMoves();
  lastMoves.clear();
  int bestScore = MINWIN;
  Move bestMove = {moves[0].first, moves[0].second};

  for (int depth = 1; !outOfTime and bestScore < MAXWIN and
                      (gravityOn or depth <= moves.size());
       depth++) {
    if (DEBUG) {
      cout << "Searching with depth " << depth << "..." << endl;
    }
    // Start using last move data from previous depth when length >= 3.
    if (lastMoves.size() >= 3) {
      if (DEBUG) {
        cout << "Prioritizing last depth's expected path." << endl;
      }
      findMoves();
    }
    Coord currentMove;
    int currentScore;
    tie(currentMove, currentScore) = startMiniMax(depth);
    if (!outOfTime and (currentScore > MINWIN or bestScore == MINWIN)) {
      bestMove = {currentMove.first, currentMove.second};
      bestScore = currentScore;
    }
    if (DEBUG and !outOfTime and (!gravityOn and depth == moves.size())) {
      cout << "Search space exhausted." << endl;
    } else if (DEBUG and bestScore == MAXWIN) {
      cout << "Winning move found." << endl;
    }
  }
  if (DEBUG and outOfTime) {
    cout << "Time limit reached." << endl << endl;
  }

  return bestMove;
}

Move AIShell::makeMove() {
  this->startTime = currentTime();
  this->stopTime = startTime + milliseconds(this->deadline) - TIME_MARGIN;
  Move m;

  if (DEBUG) {
    cout << "Expected move chain from last turn: ";
    for (int i = 0; i < lastMoves.size(); i++) {
      if (i != 0) {
        cout << "->";
      }
      cout << "(" << lastMoves[i].first << ", " << lastMoves[i].second << ")";
    }
    cout << endl << endl;
  }

  // Return center position if we're making the opening move.
  if (lastMove == Move(-1, -1)) {
    if (gravityOn) {
      m = Move(numCols / 2, 0);
    } else {
      m = Move(numCols / 2, numRows / 2);
    }
  } else {
    m = runIDS();
  }

  if (DEBUG) {
    cout << (currentTime() - startTime).count() << "ms elapsed." << endl;
    cout << "Returning " << m << "." << endl;
  }

  // Waits to return the move until time is up.
  while (!outOfTime) {
    checkTime();
  }

  return m;
}
