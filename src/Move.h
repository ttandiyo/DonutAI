#ifndef MOVE_H
#define MOVE_H
#include <iostream>

struct Move {
    int row = 0; // the row to move to.
    int col = 0; // the col to move to.
    Move();
    Move(int col, int row);
    bool operator==(const Move &other) const;
    bool operator!=(const Move &other) const;
    friend std::ostream &operator<<(std::ostream &strm, const Move &move);
};

namespace std {
template <> struct hash<Move> {
    auto operator()(Move const &item) const { // Allows use of Move in sets
        auto const h1(std::hash<int>{}(item.row));
        auto const h2(std::hash<int>{}(item.col));
        return h1 ^ (h2 << 2);
    }
};
}

// typedef UniqueQueue<Move> Moves;

#endif // MOVE_H
