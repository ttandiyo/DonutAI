#ifndef LINECOUNTER_H
#define LINECOUNTER_H

#include "AIShell.h"

class LineCounter {
    const int k;
    // int almostK;
    int minOpenStreak = 0;
    int maxOpenStreak = 0;
    int minContiguousStreak = 0;
    int maxContiguousStreak = 0;
    int winsInLine(int lineLength) const;
    int possibleWinsInLine(int lineLength) const;
    void pushMax();
    void pushMin();
    void pushContiguous();

  public:
    LineCounter(int k);
    int minPossibleWins = 0, maxPossibleWins = 0;
    int minWins = 0, maxWins = 0;
    void addPiece(int piece);
    void pushLine();
};

#endif // LINECOUNTER_H
